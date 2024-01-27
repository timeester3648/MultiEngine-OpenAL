
#include "config.h"

#include "helpers.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <limits>
#include <mutex>
#include <optional>
#include <string>
#include <system_error>

#include "almalloc.h"
#include "alnumeric.h"
#include "alspan.h"
#include "alstring.h"
#include "logging.h"
#include "strutils.h"


/* Mixing thread priority level */
int RTPrioLevel{1};

/* Allow reducing the process's RTTime limit for RTKit. */
bool AllowRTTimeLimit{true};


namespace {

std::mutex gSearchLock;

void DirectorySearch(const std::string_view path, const std::string_view ext,
    std::vector<std::string> *const results)
{
    auto as_int = [](size_t value) noexcept -> int
    { return static_cast<int>(std::min<size_t>(value, std::numeric_limits<int>::max())); };

    const auto base = static_cast<std::make_signed_t<size_t>>(results->size());

    TRACE("Searching %.*s for *%.*s\n", as_int(path.size()), path.data(), as_int(ext.size()),
        ext.data());
    try {
        for(auto&& dirent : std::filesystem::directory_iterator{std::filesystem::path(path)})
        {
            auto&& entrypath = dirent.path();
            if(!entrypath.has_extension())
                continue;

            const auto status = std::filesystem::status(entrypath);
            if(status.type() == std::filesystem::file_type::regular
                && al::case_compare(entrypath.extension().string(), ext) == 0)
                results->emplace_back(entrypath.string());
        }
    }
    catch(std::filesystem::filesystem_error &fe) {
        if(fe.code() != std::make_error_code(std::errc::no_such_file_or_directory))
            ERR("Error enumerating directory: %s\n", fe.what());
    }
    catch(std::exception& e) {
        ERR("Error enumerating directory: %s\n", e.what());
    }

    const al::span newlist{results->begin()+base, results->end()};
    std::sort(newlist.begin(), newlist.end());
    for(const auto &name : newlist)
        TRACE(" got %s\n", name.c_str());
}

} // namespace

#ifdef _WIN32

#include <cctype>
#include <shlobj.h>

const PathNamePair &GetProcBinary()
{
    static std::optional<PathNamePair> procbin;
    if(procbin) return *procbin;
#if !defined(ALSOFT_UWP)
    auto fullpath = std::vector<WCHAR>(256);
    DWORD len{GetModuleFileNameW(nullptr, fullpath.data(), static_cast<DWORD>(fullpath.size()))};
    while(len == fullpath.size())
    {
        fullpath.resize(fullpath.size() << 1);
        len = GetModuleFileNameW(nullptr, fullpath.data(), static_cast<DWORD>(fullpath.size()));
    }
    if(len == 0)
    {
        ERR("Failed to get process name: error %lu\n", GetLastError());
        procbin.emplace();
        return *procbin;
    }

    fullpath.resize(len);
    if(fullpath.back() != 0)
        fullpath.push_back(0);
#else
    auto exePath               = __wargv[0];
    if (!exePath)
    {
        ERR("Failed to get process name: error %lu\n", GetLastError());
        procbin.emplace();
        return *procbin;
    }
    std::vector<WCHAR> fullpath{exePath, exePath + wcslen(exePath) + 1};
#endif
    std::replace(fullpath.begin(), fullpath.end(), '/', '\\');
    auto sep = std::find(fullpath.rbegin()+1, fullpath.rend(), '\\');
    if(sep != fullpath.rend())
    {
        *sep = 0;
        procbin.emplace(wstr_to_utf8(fullpath.data()), wstr_to_utf8(al::to_address(sep.base())));
    }
    else
        procbin.emplace(std::string{}, wstr_to_utf8(fullpath.data()));

    TRACE("Got binary: %s, %s\n", procbin->path.c_str(), procbin->fname.c_str());
    return *procbin;
}

namespace {

#if !defined(ALSOFT_UWP) && !defined(_GAMING_XBOX)
struct CoTaskMemDeleter {
    void operator()(void *mem) const { CoTaskMemFree(mem); }
};
#endif

} // namespace

std::vector<std::string> SearchDataFiles(const char *ext, const char *subdir)
{
    std::lock_guard<std::mutex> srchlock{gSearchLock};

    /* If the path is absolute, use it directly. */
    std::vector<std::string> results;
    try {
        if(auto fpath = std::filesystem::path(subdir); fpath.is_absolute())
        {
            std::string path{fpath.make_preferred().string()};
            DirectorySearch(path, ext, &results);
            return results;
        }
    }
    catch(std::filesystem::filesystem_error &fe) {
        if(fe.code() != std::make_error_code(std::errc::no_such_file_or_directory))
            ERR("Error enumerating directory: %s\n", fe.what());
    }
    catch(std::exception& e) {
        ERR("Error enumerating directory: %s\n", e.what());
    }

    std::string path;

    /* Search the app-local directory. */
    if(auto localpath = al::getenv(L"ALSOFT_LOCAL_PATH"))
    {
        path = wstr_to_utf8(*localpath);
        std::replace(path.begin(), path.end(), '/', '\\');
        if(path.back() == '\\') path.pop_back();
    }
    else if(auto curpath = std::filesystem::current_path(); !curpath.empty())
        path = curpath.make_preferred().string();
    if(!path.empty())
        DirectorySearch(path, ext, &results);

#if !defined(ALSOFT_UWP) && !defined(_GAMING_XBOX)
    /* Search the local and global data dirs. */
    for(const auto &folderid : std::array{FOLDERID_RoamingAppData, FOLDERID_ProgramData})
    {
        std::unique_ptr<WCHAR,CoTaskMemDeleter> buffer;
        const HRESULT hr{SHGetKnownFolderPath(folderid, KF_FLAG_DONT_UNEXPAND, nullptr,
            al::out_ptr(buffer))};
        if(FAILED(hr)) continue;

        path = wstr_to_utf8(buffer.get());
        path += '\\';
        path += subdir;
        std::replace(path.begin(), path.end(), '/', '\\');

        DirectorySearch(path, ext, &results);
    }
#endif

    return results;
}

void SetRTPriority()
{
#if !defined(ALSOFT_UWP)
    if(RTPrioLevel > 0)
    {
        if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
            ERR("Failed to set priority level for thread\n");
    }
#endif
}

#else

#include <cerrno>
#include <dirent.h>
#include <unistd.h>
#ifdef __FreeBSD__
#include <sys/sysctl.h>
#endif
#ifdef __HAIKU__
#include <FindDirectory.h>
#endif
#ifdef HAVE_PROC_PIDPATH
#include <libproc.h>
#endif
#if defined(HAVE_PTHREAD_SETSCHEDPARAM) && !defined(__OpenBSD__)
#include <pthread.h>
#include <sched.h>
#endif
#ifdef HAVE_RTKIT
#include <sys/resource.h>

#include "dbus_wrap.h"
#include "rtkit.h"
#ifndef RLIMIT_RTTIME
#define RLIMIT_RTTIME 15
#endif
#endif

using namespace std::string_view_literals;

const PathNamePair &GetProcBinary()
{
    static std::optional<PathNamePair> procbin;
    if(procbin) return *procbin;

    std::vector<char> pathname;
#ifdef __FreeBSD__
    size_t pathlen;
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
    if(sysctl(mib, 4, nullptr, &pathlen, nullptr, 0) == -1)
        WARN("Failed to sysctl kern.proc.pathname: %s\n",
            std::generic_category().message(errno).c_str());
    else
    {
        pathname.resize(pathlen + 1);
        sysctl(mib, 4, pathname.data(), &pathlen, nullptr, 0);
        pathname.resize(pathlen);
    }
#endif
#ifdef HAVE_PROC_PIDPATH
    if(pathname.empty())
    {
        char procpath[PROC_PIDPATHINFO_MAXSIZE]{};
        const pid_t pid{getpid()};
        if(proc_pidpath(pid, procpath, sizeof(procpath)) < 1)
            ERR("proc_pidpath(%d, ...) failed: %s\n", pid,
                std::generic_category().message(errno).c_str());
        else
            pathname.insert(pathname.end(), procpath, procpath+strlen(procpath));
    }
#endif
#ifdef __HAIKU__
    if(pathname.empty())
    {
        char procpath[PATH_MAX];
        if(find_path(B_APP_IMAGE_SYMBOL, B_FIND_PATH_IMAGE_PATH, NULL, procpath, sizeof(procpath)) == B_OK)
            pathname.insert(pathname.end(), procpath, procpath+strlen(procpath));
    }
#endif
#ifndef __SWITCH__
    if(pathname.empty())
    {
        const std::array SelfLinkNames{
            "/proc/self/exe"sv,
            "/proc/self/file"sv,
            "/proc/curproc/exe"sv,
            "/proc/curproc/file"sv,
        };

        std::string selfname{};
        for(const std::string_view name : SelfLinkNames)
        {
            try {
                auto path = std::filesystem::read_symlink(name);
                if(!path.empty())
                {
                    selfname = path.u8string();
                    break;
                }
            }
            catch(std::filesystem::filesystem_error& fe) {
                if(fe.code() != std::make_error_code(std::errc::no_such_file_or_directory))
                    WARN("Failed to readlink %.*s: %s\n", static_cast<int>(name.size()),
                        name.data(), fe.what());
            }
            catch(...) {
            }
        }

        if(!selfname.empty())
        {
            pathname.resize(selfname.size());
            std::copy(selfname.cbegin(), selfname.cend(), pathname.begin());
        }
    }
#endif
    while(!pathname.empty() && pathname.back() == 0)
        pathname.pop_back();

    auto sep = std::find(pathname.crbegin(), pathname.crend(), '/');
    if(sep != pathname.crend())
        procbin.emplace(std::string(pathname.cbegin(), sep.base()-1),
            std::string(sep.base(), pathname.cend()));
    else
        procbin.emplace(std::string{}, std::string(pathname.cbegin(), pathname.cend()));

    TRACE("Got binary: \"%s\", \"%s\"\n", procbin->path.c_str(), procbin->fname.c_str());
    return *procbin;
}

std::vector<std::string> SearchDataFiles(const char *ext, const char *subdir)
{
    std::lock_guard<std::mutex> srchlock{gSearchLock};

    std::vector<std::string> results;
    try {
        if(auto fpath = std::filesystem::u8path(subdir); fpath.is_absolute())
        {
            DirectorySearch(subdir, ext, &results);
            return results;
        }
    }
    catch(std::filesystem::filesystem_error &fe) {
        if(fe.code() != std::make_error_code(std::errc::no_such_file_or_directory))
            ERR("Error enumerating directory: %s\n", fe.what());
    }
    catch(std::exception& e) {
        ERR("Error enumerating directory: %s\n", e.what());
    }

    /* Search the app-local directory. */
    if(auto localpath = al::getenv("ALSOFT_LOCAL_PATH"))
        DirectorySearch(*localpath, ext, &results);
    else if(auto curpath = std::filesystem::current_path(); !curpath.empty())
        DirectorySearch(curpath.u8string(), ext, &results);

    // Search local data dir
    if(auto datapath = al::getenv("XDG_DATA_HOME"))
    {
        std::string &path = *datapath;
        if(path.back() != '/')
            path += '/';
        path += subdir;
        DirectorySearch(path, ext, &results);
    }
    else if(auto homepath = al::getenv("HOME"))
    {
        std::string &path = *homepath;
        if(path.back() == '/')
            path.pop_back();
        path += "/.local/share/";
        path += subdir;
        DirectorySearch(path, ext, &results);
    }

    // Search global data dirs
    std::string datadirs{al::getenv("XDG_DATA_DIRS").value_or("/usr/local/share/:/usr/share/")};

    size_t curpos{0u};
    while(curpos < datadirs.size())
    {
        size_t nextpos{datadirs.find(':', curpos)};

        std::string path{(nextpos != std::string::npos) ?
            datadirs.substr(curpos, nextpos++ - curpos) : datadirs.substr(curpos)};
        curpos = nextpos;

        if(path.empty()) continue;
        if(path.back() != '/')
            path += '/';
        path += subdir;

        DirectorySearch(path, ext, &results);
    }

#ifdef ALSOFT_INSTALL_DATADIR
    // Search the installation data directory
    {
        std::string path{ALSOFT_INSTALL_DATADIR};
        if(!path.empty())
        {
            if(path.back() != '/')
                path += '/';
            path += subdir;
            DirectorySearch(path, ext, &results);
        }
    }
#endif

    return results;
}

namespace {

bool SetRTPriorityPthread(int prio [[maybe_unused]])
{
    int err{ENOTSUP};
#if defined(HAVE_PTHREAD_SETSCHEDPARAM) && !defined(__OpenBSD__)
    /* Get the min and max priority for SCHED_RR. Limit the max priority to
     * half, for now, to ensure the thread can't take the highest priority and
     * go rogue.
     */
    int rtmin{sched_get_priority_min(SCHED_RR)};
    int rtmax{sched_get_priority_max(SCHED_RR)};
    rtmax = (rtmax-rtmin)/2 + rtmin;

    struct sched_param param{};
    param.sched_priority = clampi(prio, rtmin, rtmax);
#ifdef SCHED_RESET_ON_FORK
    err = pthread_setschedparam(pthread_self(), SCHED_RR|SCHED_RESET_ON_FORK, &param);
    if(err == EINVAL)
#endif
        err = pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    if(err == 0) return true;
#endif
    WARN("pthread_setschedparam failed: %s (%d)\n", std::generic_category().message(err).c_str(),
        err);
    return false;
}

bool SetRTPriorityRTKit(int prio [[maybe_unused]])
{
#ifdef HAVE_RTKIT
    if(!HasDBus())
    {
        WARN("D-Bus not available\n");
        return false;
    }
    dbus::Error error;
    dbus::ConnectionPtr conn{dbus_bus_get(DBUS_BUS_SYSTEM, &error.get())};
    if(!conn)
    {
        WARN("D-Bus connection failed with %s: %s\n", error->name, error->message);
        return false;
    }

    /* Don't stupidly exit if the connection dies while doing this. */
    dbus_connection_set_exit_on_disconnect(conn.get(), false);

    int nicemin{};
    int err{rtkit_get_min_nice_level(conn.get(), &nicemin)};
    if(err == -ENOENT)
    {
        err = std::abs(err);
        ERR("Could not query RTKit: %s (%d)\n", std::generic_category().message(err).c_str(), err);
        return false;
    }
    int rtmax{rtkit_get_max_realtime_priority(conn.get())};
    TRACE("Maximum real-time priority: %d, minimum niceness: %d\n", rtmax, nicemin);

    auto limit_rttime = [](DBusConnection *c) -> int
    {
        using ulonglong = unsigned long long;
        long long maxrttime{rtkit_get_rttime_usec_max(c)};
        if(maxrttime <= 0) return static_cast<int>(std::abs(maxrttime));
        const ulonglong umaxtime{static_cast<ulonglong>(maxrttime)};

        struct rlimit rlim{};
        if(getrlimit(RLIMIT_RTTIME, &rlim) != 0)
            return errno;

        TRACE("RTTime max: %llu (hard: %llu, soft: %llu)\n", umaxtime,
            static_cast<ulonglong>(rlim.rlim_max), static_cast<ulonglong>(rlim.rlim_cur));
        if(rlim.rlim_max > umaxtime)
        {
            rlim.rlim_max = static_cast<rlim_t>(std::min<ulonglong>(umaxtime,
                std::numeric_limits<rlim_t>::max()));
            rlim.rlim_cur = std::min(rlim.rlim_cur, rlim.rlim_max);
            if(setrlimit(RLIMIT_RTTIME, &rlim) != 0)
                return errno;
        }
        return 0;
    };
    if(rtmax > 0)
    {
        if(AllowRTTimeLimit)
        {
            err = limit_rttime(conn.get());
            if(err != 0)
                WARN("Failed to set RLIMIT_RTTIME for RTKit: %s (%d)\n",
                    std::generic_category().message(err).c_str(), err);
        }

        /* Limit the maximum real-time priority to half. */
        rtmax = (rtmax+1)/2;
        prio = clampi(prio, 1, rtmax);

        TRACE("Making real-time with priority %d (max: %d)\n", prio, rtmax);
        err = rtkit_make_realtime(conn.get(), 0, prio);
        if(err == 0) return true;

        err = std::abs(err);
        WARN("Failed to set real-time priority: %s (%d)\n",
            std::generic_category().message(err).c_str(), err);
    }
    /* Don't try to set the niceness for non-Linux systems. Standard POSIX has
     * niceness as a per-process attribute, while the intent here is for the
     * audio processing thread only to get a priority boost. Currently only
     * Linux is known to have per-thread niceness.
     */
#ifdef __linux__
    if(nicemin < 0)
    {
        TRACE("Making high priority with niceness %d\n", nicemin);
        err = rtkit_make_high_priority(conn.get(), 0, nicemin);
        if(err == 0) return true;

        err = std::abs(err);
        WARN("Failed to set high priority: %s (%d)\n",
            std::generic_category().message(err).c_str(), err);
    }
#endif /* __linux__ */

#else

    WARN("D-Bus not supported\n");
#endif
    return false;
}

} // namespace

void SetRTPriority()
{
    if(RTPrioLevel <= 0)
        return;

    if(SetRTPriorityPthread(RTPrioLevel))
        return;
    if(SetRTPriorityRTKit(RTPrioLevel))
        return;
}

#endif

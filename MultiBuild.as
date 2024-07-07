void main(MultiBuild::Workspace& workspace) {	
	auto project = workspace.create_project(".");
	auto properties = project.properties();

	properties.name("OpenAL");
	properties.binary_object_kind(MultiBuild::BinaryObjectKind::eSharedLib);
	properties.license("./COPYING");
	properties.tags("use_header_only_mle");

	properties.project_includes("Intrinsics");

	project.include_own_required_includes(true);
	project.add_required_project_include({
		"./include"
	});
	
	properties.include_directories({
		"./",
		"./al",
		"./alc",
		"./core",
		"./common"
	});
	
	properties.defines("HAVE_SSE_INTRINSICS=(INTRINSICS_LEVEL_SSE != 0)");

	properties.files({
		"./al/**.h",
		"./al/**.cpp",

		"./alc/**.h",
		"./alc/**.cpp",

		"./core/**.h",
		"./core/**.cpp",

		"./common/**.h",
		"./common/**.cpp",

		"./include/**.h"
	});

	{
		MultiBuild::ScopedFilter _(workspace, "project.compiler:VisualCpp");
		properties.disable_warnings({ "5030", "4065", "4834", "4267", "4067", "4244", "4018", "4804" });
	}

	{
		MultiBuild::ScopedFilter _(workspace, "config.platform:Windows");
		properties.defines({
			"AL_ALEXT_PROTOTYPES", 
			"RESTRICT=__restrict", 
			"_CRT_SECURE_NO_WARNINGS", 
			"AL_API=__declspec(dllexport)", 
			"ALC_API=__declspec(dllexport)"
		});

		properties.exclude_files({
			"./core/rtkit.cpp",
			"./core/dbus_wrap.cpp",
			"./core/mixer/mixer_neon.cpp",

			"./alc/backends/oss.cpp",
			"./alc/backends/jack.cpp",
			"./alc/backends/alsa.cpp",
			"./alc/backends/oboe.cpp",
			"./alc/backends/sdl2.cpp",
			"./alc/backends/sndio.cpp",
			"./alc/backends/opensl.cpp",
			"./alc/backends/solaris.cpp",
			"./alc/backends/pipewire.cpp",
			"./alc/backends/coreaudio.cpp",
			"./alc/backends/portaudio.cpp",
			"./alc/backends/pulseaudio.cpp"
		});
	}
}
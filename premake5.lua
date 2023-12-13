include "../../premake/common_premake_defines.lua"

project "OpenAL"
	kind "SharedLib"
	language "C++"
	cppdialect "C++latest"
	cdialect "C17"
	targetname "%{prj.name}"
	inlining "Auto"
	tags { "use_header_only_mle" }

	includedirs {
		-- Note: not separate static needed, since only defines are used
		"%{IncludeDir.mle}",
		"%{IncludeDir.intrinsics}",

		"./",
		"./al",
		"./alc",
		"./core",
		"./common",
		"./include"
	}

	files {
		"./al/**.h",
		"./al/**.cpp",

		"./alc/**.h",
		"./alc/**.cpp",

		"./core/**.h",
		"./core/**.cpp",

		"./common/**.h",
		"./common/**.cpp",

		"./include/**.h"
	}

	defines {
		"HAVE_SSE_INTRINSICS=(INTRINSICS_LEVEL_SSE != 0)"
	}

	filter "toolset:msc"
		disablewarnings { "5030", "4065", "4834", "4267", "4067", "4244", "4018", "4804" }

 	filter "system:windows"
		links { "Winmm" }
		defines { "AL_ALEXT_PROTOTYPES", "RESTRICT=__restrict", "_CRT_SECURE_NO_WARNINGS", "AL_API=__declspec(dllexport)", "ALC_API=__declspec(dllexport)" }
		excludes { "./core/rtkit.cpp",
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
				   "./alc/backends/pulseaudio.cpp" }
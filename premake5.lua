include "../../premake/common_premake_defines.lua"

project "OpenAL"
	kind "StaticLib"
	language "C++"
	cppdialect "C++latest"
	cdialect "C17"
	targetname "%{prj.name}"
	inlining "Auto"

	includedirs {
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
		"AL_LIBTYPE_STATIC",
		"HAVE_SSE_INTRINSICS=(INTRINSICS_LEVEL_SSE != 0)"
	}

	filter "toolset:msc"
		disablewarnings { "5030", "4065", "4834", "4267", "4067", "4244", "4018" }

 	filter "system:windows"
		defines { "AL_ALEXT_PROTOTYPES", "AL_BUILD_LIBRARY", "RESTRICT=__restrict", "_CRT_SECURE_NO_WARNINGS" }
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
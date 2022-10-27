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
		"AL_LIBTYPE_STATIC"
	}

 	filter "system:windows"
		disablewarnings { "5030", "4065", "4834" }
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

 	filter "configurations:Debug"
		defines { "MLE_DEBUG_BUILD", "DEBUG" }
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines { "MLE_RELEASE_BUILD", "NDEBUG" }
		flags { "LinkTimeOptimization" }
		runtime "Release"
		optimize "speed"
		intrinsics "on"

	filter "configurations:Distribution"
		defines {  "MLE_DISTRIBUTION_BUILD", "NDEBUG" }
		flags { "LinkTimeOptimization" }
		runtime "Release"
		optimize "speed"
		intrinsics "on"
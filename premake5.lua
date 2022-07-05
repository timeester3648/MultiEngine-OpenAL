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

		"./include",

		"./src",
		"./src/al",
		"./src/alc",
		"./src/core",
		"./src/common"
	}

	files {
		"./**.h",
		"./**.cpp"
	}

	defines {
		"AL_LIBTYPE_STATIC"
	}

 	filter "system:windows"
		disablewarnings { "5030", "4065", "4834" }
		defines { "AL_ALEXT_PROTOTYPES", "AL_BUILD_LIBRARY", "RESTRICT=__restrict", "_CRT_SECURE_NO_WARNINGS" }
		excludes { "./src/core/rtkit.cpp",
				   "./src/core/dbus_wrap.cpp",
				   "./src/core/mixer/mixer_neon.cpp",

				   "./src/alc/backends/oss.cpp",
				   "./src/alc/backends/jack.cpp",
				   "./src/alc/backends/alsa.cpp",
				   "./src/alc/backends/oboe.cpp",
				   "./src/alc/backends/sdl2.cpp",
				   "./src/alc/backends/sndio.cpp",
				   "./src/alc/backends/opensl.cpp",
				   "./src/alc/backends/solaris.cpp",
				   "./src/alc/backends/pipewire.cpp",
				   "./src/alc/backends/coreaudio.cpp",
				   "./src/alc/backends/portaudio.cpp",
				   "./src/alc/backends/pulseaudio.cpp" }

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
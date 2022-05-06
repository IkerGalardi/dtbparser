workspace "Userspace Slab"
    configurations { "Debug", "Release" }

    project "dtbparser"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++20"

        targetdir "."
        objdir "bin/"

        files { "src/**.cc", "src/**.hh" }
        includedirs { "src/" }

        filter "configurations:Debug"
            symbols "on"
            optimize "off"
        filter "configurations:Release"
            optimize "on"
            symbols "off"
            runtime "Release"

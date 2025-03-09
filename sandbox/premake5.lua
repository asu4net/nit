project "sandbox"

    kind          "ConsoleApp"
    language      "C++"
    cppdialect    "C++20"
    targetdir     (binariesdir)
    objdir        (intermediatesdir)
    pchheader     "app_pch.h"
    pchsource     "src/app_pch.cpp"
    forceincludes { "app_pch.h" }
    
    defines
    {
    }

    disablewarnings 
    {
    }

    includedirs 
    {
        "../nit/src",
        "src",
        "../3rd/imgui/src",
        "../3rd/yaml/include"
    }
    
    links
    {
        "nit"
    }

    files { "src/**.h", "src/**.cpp" }

    filter "configurations:Debug"
        symbols "On"
        runtime "Debug"
        defines "NIT_DEBUG"

    filter "configurations:Release"
        optimize "On"
        runtime "Release"
        defines "NIT_RELEASE"

    filter "configurations:Dist"
		runtime "Release"
		optimize "On"
        defines "NIT_DIST"

    filter "system:windows"
        systemversion "latest"
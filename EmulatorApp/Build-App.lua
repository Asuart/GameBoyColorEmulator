project "EmulatorApp"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "build/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp" }

   includedirs
   {
      "Source",
      "../dependencies/PixieUI/include/",
      "../dependencies/PixieUI/dependencies/GLFW/include/",
      "../dependencies/PixieUI/dependencies/GLAD/include/",
	  "../EmulatorCore/Source"
   }

   links
   {
      "EmulatorCore",
      "PixieUI",
	  "glfw3.lib"
   }

   libdirs { "../dependencies/PixieUI/dependencies/GLFW/lib-vc2022-64/" }

   targetdir ("../build/" .. OutputDir .. "/%{prj.name}")
   objdir ("../build/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { "WINDOWS" }

   filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"

   filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"

   filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"
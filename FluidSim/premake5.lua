project "FluidSim"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/Stengine/vendor/spdlog/include",
		"%{wks.location}/Stengine/src",
		"%{wks.location}/Stengine/vendor",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGuizmo}"
	}

  libdirs
  {
    "%{LibraryDir.VulkanSDK}"
  }

	links
	{
		"Stengine",
		"GLFW",
		"Glad",
		"ImGui",
		"Box2D",
		"yaml-cpp",
	}

--[[ --------------------- Linux --------------------- ]]--

	filter "system:linux"
		systemversion "latest"
		defines { "ST_PLATFORM_LINUX" }

    buildoptions { "`pkg-config --cflags gtk+-3.0`" }
    linkoptions { "`pkg-config --libs gtk+-3.0`" }
    
    libdirs
    {
      "%{VULKAN_SDK}/lib"
    }

		links
		{
      "gtk-3", "glib-2.0", "gobject-2.0",
      "shaderc_combined",
      "spirv-cross-core",
      "spirv-cross-glsl",
		}
		
		postbuildcommands { "cp -r \"%{wks.location}/%{prj.name}/assets\" \"%{wks.location}/bin/" .. outputdir .. "/%{prj.name}/assets\"" }

--[[ -------------------- Windows -------------------- ]]--

	filter "system:windows"
		systemversion "latest"
		defines { "ST_PLATFORM_WINDOWS" }
		
		postbuildcommands { "xcopy \"%{wks.location}/%{prj.name}/assets\" \"%{wks.location}/bin/" .. outputdir .. "/%{prj.name}/assets\" /E /Y /I" }

	filter { "configurations:Debug", "system:windows" }
		links
		{
			"%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib",
			"%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib",
			"%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
		}

		postbuildcommands
		{
			"copy \"%{wks.location}Stengine\\vendor\\VulkanSDK\\bin\\shaderc_sharedd.dll\" \"%{wks.location}%{prj.name}\"",
			"copy \"%{wks.location}Stengine\\vendor\\VulkanSDK\\bin\\shaderc_sharedd.dll\" \"%{wks.location}\\bin\\" .. outputdir .. "\\%{prj.name}\"",
		}
		
	filter { "configurations:Release", "system:windows" }
		links
		{
			"%{LibraryDir.VulkanSDK}/shaderc_shared.lib",
			"%{LibraryDir.VulkanSDK}/spirv-cross-core.lib",
			"%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"
		}

	filter { "configurations:Dist", "system:windows" }
		links
		{
			"%{LibraryDir.VulkanSDK}/shaderc_shared.lib",
			"%{LibraryDir.VulkanSDK}/spirv-cross-core.lib",
			"%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"
		}
		
--[[ ------------------ Configurations ------------------ ]]--

	filter "configurations:Debug"
		defines "ST_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "ST_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "ST_DIST"
		runtime "Release"
		optimize "On"

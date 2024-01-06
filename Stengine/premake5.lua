project "Stengine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "stpch.h"
	pchsource "src/stpch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",

		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",

		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",

		"vendor/ImGuizmo/ImGuizmo.h",
		"vendor/ImGuizmo/ImGuizmo.cpp",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"YAML_CPP_STATIC_DEFINE"
	}
 
	includedirs
	{
		"src",
		"vendor/spdlog/include",

		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.VulkanSDK}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"Box2D",
		"yaml-cpp",
		"opengl32.lib"
	}

	filter "files:vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }

--[[ --------------------- Linux --------------------- ]]--

	filter "system:linux"
		systemversion "latest"
		defines { "ST_PLATFORM_LINUX" }

		links 
		{ 			
			"%{LibraryDir.VulkanSDK}/libshaderc_shared.so",
			"%{LibraryDir.VulkanSDK}/libspirv-cross-core.a",
			"%{LibraryDir.VulkanSDK}/libspirv-cross-glsl.a",
			"gtk-3",
		}
		
        buildoptions { "`pkg-config --cflags gtk+-3.0`" }
        linkoptions { "`pkg-config --libs gtk+-3.0`" }

--[[ -------------------- Windows -------------------- ]]--

	filter "system:windows"
		systemversion "latest"
		defines { "ST_PLATFORM_WINDOWS" }

	filter { "configurations:Debug", "system:windows" }
		links
		{
			"%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib",
			"%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib",
			"%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
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
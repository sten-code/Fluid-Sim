workspace "FluidSim"
	architecture "x86_64"
	startproject "FluidSim"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

-- The output directory based on the configurations
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["GLFW"] 			= "%{wks.location}/Stengine/vendor/GLFW/include"
IncludeDir["Glad"] 			= "%{wks.location}/Stengine/vendor/Glad/include"
IncludeDir["ImGui"] 		= "%{wks.location}/Stengine/vendor/imgui"
IncludeDir["ImGuizmo"] 		= "%{wks.location}/Stengine/vendor/ImGuizmo"
IncludeDir["Box2D"]			= "%{wks.location}/Stengine/vendor/Box2D/include"
IncludeDir["glm"] 			= "%{wks.location}/Stengine/vendor/glm"
IncludeDir["entt"] 			= "%{wks.location}/Stengine/vendor/entt/include"
IncludeDir["stb_image"] 	= "%{wks.location}/Stengine/vendor/stb_image"
IncludeDir["yaml_cpp"] 		= "%{wks.location}/Stengine/vendor/yaml-cpp/include"
IncludeDir["VulkanSDK"] 	= "%{VULKAN_SDK}/include"

LibraryDir = {}
LibraryDir["VulkanSDK"] 		= "%{VULKAN_SDK}/lib"
LibraryDir["VulkanSDK_Debug"] 	= "%{wks.location}/Stengine/vendor/VulkanSDK/lib"

-- Create a solution folder inside visual studio
group "Dependencies"
	include "Stengine/vendor/GLFW"
	include "Stengine/vendor/Glad"
	include "Stengine/vendor/imgui"
	include "Stengine/vendor/Box2D"
	include "Stengine/vendor/yaml-cpp"
group "" -- End the solution folder here

-- Include the premake5.lua files inside these project folders so that they get added to the build system
include "Stengine"
include "FluidSim"
#include <Stengine.h>
#include <Stengine/Core/EntryPoint.h>

#include "SimulationLayer.h"

#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

namespace Sten
{
	class FluidSim : public Application
	{
	public:
		FluidSim(ApplicationCommandLineArgs args)
			: Application(WindowProps("Fluid Simulation", 1280, 720), args)
		{
			PushLayer(new SimulationLayer());
		}

		~FluidSim()
		{
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		return new FluidSim(args);
	}
}
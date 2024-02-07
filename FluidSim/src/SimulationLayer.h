#pragma once

#include <Stengine.h>
#include "Stengine/Renderer/EditorCamera.h"

#include "NavierStokesFluid.h"
#include "ParticleFluid.h"
#include "VerletIntegration.h"

namespace Sten
{
	class SimulationLayer : public Layer
	{
	public:
		SimulationLayer();
		virtual ~SimulationLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;
	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMousePressed(MouseButtonPressedEvent& e);
	private:
		Ref<Framebuffer> m_Framebuffer;
		Ref<OrthographicCamera> m_Camera;
		NavierStokesFluid m_NavierStokesFluid;
		ParticleFluid m_ParticleFluid;
		VerletIntegration m_VerletIntegration;

		enum
		{
			NavierStokes, Particle, Verlet
		} m_ActiveFluid = Verlet;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportOffset = { 0.0f, 0.0f };
		float m_Fps = 0.0f;
	};
}
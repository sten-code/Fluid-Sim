#include "SimulationLayer.h"

#include "Stengine/Scene/SceneSerializer.h"
#include "Stengine/Utils/PlatformUtils.h"
#include "Stengine/Math/Math.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <chrono>

namespace Sten
{
	SimulationLayer::SimulationLayer()
		: Layer("SimulationLayer"), m_NavierStokesFluid(256, 256, 3, 0, 0.0000001, 1)
	{
		//Application::Get().GetWindow().SetVSync(true);
	}

	void SimulationLayer::OnAttach()
	{
		ST_PROFILE_FUNCTION();

		FramebufferSpecification spec;
		spec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
		spec.Width = 1280;
		spec.Height = 720;
		m_Framebuffer = Framebuffer::Create(spec);

		m_Camera = CreateRef<OrthographicCamera>(-1280 / 2.0f, 1280 / 2.0f, -720 / 2.0f, 720 / 2.0f);
	}

	void SimulationLayer::OnDetach()
	{
		ST_PROFILE_FUNCTION();
	}

	void SimulationLayer::OnUpdate(Timestep ts)
	{
		ST_PROFILE_FUNCTION();
		m_Fps = 1.0f / ts;

		// Resize
		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_Camera->SetProjection(-m_ViewportSize.x / 2.0f, m_ViewportSize.x / 2.0f, -m_ViewportSize.y / 2.0f, m_ViewportSize.y / 2.0f);
			if (m_ActiveFluid == NavierStokes)
				m_NavierStokesFluid.Resize(m_ViewportSize.x / m_NavierStokesFluid.GetScale(), m_ViewportSize.y / m_NavierStokesFluid.GetScale());
		}

		if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
		{
			if (m_ActiveFluid == NavierStokes)
			{
				glm::vec2 pos = (Input::GetMousePosition() - m_ViewportOffset - m_ViewportSize / 2.0f) / (float)m_NavierStokesFluid.GetScale() + glm::vec2{ m_NavierStokesFluid.GetWidth() / 2.0f, m_NavierStokesFluid.GetHeight() / 2.0f };
				m_NavierStokesFluid.AddDensity(pos.x, m_NavierStokesFluid.GetHeight() - pos.y, 5, 5.0f);
			}
		}

		if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
		{
			if (m_ActiveFluid == NavierStokes)
			{
				static glm::vec2 prev = { 0, 0 };
				glm::vec2 pos = (Input::GetMousePosition() - m_ViewportOffset - m_ViewportSize / 2.0f) / (float)m_NavierStokesFluid.GetScale() + glm::vec2{ m_NavierStokesFluid.GetWidth() / 2.0f, m_NavierStokesFluid.GetHeight() / 2.0f };
				glm::vec2 amount = (pos - prev);
				m_NavierStokesFluid.AddVelocity(pos.x, m_NavierStokesFluid.GetHeight() - pos.y, 10, amount.x, -amount.y);
				prev = pos;
			}
		}

		if (m_ActiveFluid == NavierStokes)
			m_NavierStokesFluid.Step(ts);
		else if (m_ActiveFluid == Particle)
			m_ParticleFluid.Step(m_ViewportOffset, m_ViewportSize, ts);
		else if (m_ActiveFluid == Verlet)
			m_VerletIntegration.Step(m_ViewportOffset, m_ViewportSize, ts);

		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		Renderer2D::BeginScene(m_Camera->GetViewProjectionMatrix());

		if (m_ActiveFluid == NavierStokes)
			m_NavierStokesFluid.Render();
		else if (m_ActiveFluid == Particle)
			m_ParticleFluid.Render();
		else if (m_ActiveFluid == Verlet)
			m_VerletIntegration.Render(m_ViewportOffset, m_ViewportSize);

		Renderer2D::EndScene();

		// End
		m_Framebuffer->Unbind();
	}

	void SimulationLayer::OnImGuiRender()
	{
		static bool dockingspace_show = true;

		// Ensure that the docking space is taking up the entire window
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		// Window settings of the main docking space
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		
		// Select all of the styles of the docking space
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);				// Rounding of the main docking space
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);				// The border thickness of the main docking space
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));	// 0 Padding of the docking space relative to the window

		ImGui::Begin("Docking Space", &dockingspace_show, window_flags);

		// Pop all of the pushed style vars
		ImGui::PopStyleVar(3);

		// Submit the docking space
		ImGuiID dockspace_id = ImGui::GetID("DockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

		// ------------------- ImGui -------------------

		if (m_ActiveFluid == Particle)
			m_ParticleFluid.OnImGuiRender();

		ImGui::Begin("Statistics");

		auto stats = Renderer2D::GetStats();
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		ImGui::End();

		ImGui::Begin("FPS");
		ImGui::LabelText("FPS", "%.2f", m_Fps);

		ImGui::End();

		// --------------------- Viewport ------------------------

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
		ImGui::Begin("Viewport");

		ImVec2 main = ImGui::GetMainViewport()->Pos;
		ImVec2 pos = ImGui::GetWindowPos();

		m_ViewportOffset = glm::vec2{pos.x, pos.y} - glm::vec2{main.x, main.y};

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportSize.x, viewportSize.y };

		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		
		ImGui::End(); // Viewport
		ImGui::PopStyleVar();

		// End the docking space
		ImGui::End();
	}

	void SimulationLayer::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(ST_BIND_EVENT_FN(OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(ST_BIND_EVENT_FN(OnMousePressed));
	}

	bool SimulationLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetRepeatCount() > 0)
			return false;

		bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
		switch (e.GetKeyCode())
		{
		case Key::N:
			m_ActiveFluid = NavierStokes;
			break;
		case Key::P:
			m_ActiveFluid = Particle;
			break;
		case Key::V:
			m_ActiveFluid = Verlet;
			break;
		}

    return false;
	}

	bool SimulationLayer::OnMousePressed(MouseButtonPressedEvent& e)
	{
		return false;
	}
}

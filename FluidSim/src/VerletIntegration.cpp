#include "VerletIntegration.h"

glm::vec4 MapSpeedToColor(const glm::vec2& velocity, float maxSpeed);

VerletIntegration::VerletIntegration()
{
	m_SubSteps = 8;

	int width = 40;
	int height = 40;
	int spacing = 10;
	for (int x = 0; x < width; x++)
	for (int y = 0; y < height; y++)
	{
		AddObject({ x * spacing - (width * spacing) / 2, y * spacing - (height * spacing) / 2 }, 3);
	}
}

void VerletIntegration::Step(glm::vec2 viewportOffset, glm::vec2 viewportSize, float dt)
{
	m_FrameDt = 1.0f/165.0f;
	m_Time += m_FrameDt;

	const float step_dt = m_FrameDt / (float)m_SubSteps;
	for (uint32_t i{ m_SubSteps }; i--;) {
		ApplyGravity();
		CheckCollisions(step_dt);
		ApplyConstraint();
		UpdateObjects(step_dt);
	}
}

void VerletIntegration::Render(glm::vec2 viewportOffset, glm::vec2 viewportSize)
{
	if (Sten::Input::IsMouseButtonPressed(Sten::Mouse::ButtonLeft))
	{
		glm::vec2 pos = Sten::Input::GetMousePosition() - viewportOffset - viewportSize / 2.0f;
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), { pos.x, -pos.y, 0.0f })
			* glm::scale(glm::mat4(1.0f), { 10, 10, 1.0f });
		Sten::Renderer2D::DrawCircle({ transform });

		for (VerletObject& obj : m_Objects)
		{
			glm::vec2 diff = pos - obj.Position;
			float dist2 = diff.x * diff.x + diff.y * diff.y;
			if (dist2 < 10000)
			{
				obj.addVelocity(diff / 5.0f, 1.0f/165.0f);
			}
		}
	}

	for (VerletObject& obj : m_Objects)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), { obj.Position.x, -obj.Position.y, 0.0f })
			* glm::scale(glm::mat4(1.0f), { obj.Radius * 2.0f, obj.Radius * 2.0f, 1.0f });
		glm::vec4 color = MapSpeedToColor(obj.getVelocity(m_FrameDt), 50.0f);
		Sten::Renderer2D::DrawCircle({ transform, color });
	}
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), { m_ConstraintCenter.x, -m_ConstraintCenter.y, 0.0f }) *
		glm::scale(glm::mat4(1.0f), { m_ConstraintRadius * 2.0f, m_ConstraintRadius * 2.0f, 1.0f });
	Sten::Renderer2D::DrawCircle({ transform, {.3f, .3f, .3f, 1} });
}

void VerletIntegration::ApplyGravity()
{
	for (auto& obj : m_Objects)
		obj.accelerate(m_Gravity);
}

void VerletIntegration::CheckCollisions(float dt)
{
	const float response_coef = 0.75f;
	const uint64_t objects_count = m_Objects.size();

	// Iterate on all objects
	for (uint64_t i = 0; i < objects_count; ++i) {
		VerletObject& object_1 = m_Objects[i];

		// Iterate on object involved in new collision pairs
		for (uint64_t k{ i + 1 }; k < objects_count; ++k) {
			VerletObject& object_2 = m_Objects[k];
			const glm::vec2 v = object_1.Position - object_2.Position;
			const float dist2 = v.x * v.x + v.y * v.y;
			const float min_dist = object_1.Radius + object_2.Radius;

			// Check overlapping
			if (dist2 < min_dist * min_dist)
			{
				const float dist = std::sqrt(dist2);
				const glm::vec2 n = v / dist;
				const float mass_ratio_1 = object_1.Radius / (object_1.Radius + object_2.Radius);
				const float mass_ratio_2 = object_2.Radius / (object_1.Radius + object_2.Radius);
				const float delta = 0.5f * response_coef * (dist - min_dist);

				// Update positions
				object_1.Position -= n * (mass_ratio_2 * delta);
				object_2.Position += n * (mass_ratio_1 * delta);
			}
		}
	}
}

void VerletIntegration::ApplyConstraint()
{
	for (auto& obj : m_Objects)
	{
		const glm::vec2 v = m_ConstraintCenter - obj.Position;
		const float dist = std::sqrt(v.x * v.x + v.y * v.y);
		if (dist > (m_ConstraintRadius - obj.Radius))
		{
			const glm::vec2 n = v / dist;
			obj.Position = m_ConstraintCenter - n * (m_ConstraintRadius - obj.Radius);
		}
	}
}

void VerletIntegration::UpdateObjects(float dt)
{
	for (auto& obj : m_Objects)
		obj.update(dt);
}
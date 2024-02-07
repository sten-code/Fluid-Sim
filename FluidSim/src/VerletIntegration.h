#pragma once

#include <Stengine.h>

#include <glm/glm.hpp>
#include <vector>

struct VerletObject
{
    glm::vec2 Position;
    glm::vec2 PositionLast;
    glm::vec2 Acceleration;
    float     Radius = 10.0f;
    glm::vec4 Color = { 1, 1, 1, 1 };

    VerletObject() = default;
    VerletObject(glm::vec2 position, float radius)
        : Position(position), PositionLast(position), Acceleration({ 0.0f, 0.0f }), Radius(radius)
    {
    }

    void update(float dt)
    {
        // Compute how much we moved
        const glm::vec2 displacement = Position - PositionLast;
        // Update position
        PositionLast = Position;
        Position = Position + displacement + Acceleration * (dt * dt);
        // Reset acceleration
        Acceleration = {};
    }

    void accelerate(glm::vec2 a)
    {
        Acceleration += a;
    }

    void setVelocity(glm::vec2 v, float dt)
    {
        PositionLast = Position - (v * dt);
    }

    void addVelocity(glm::vec2 v, float dt)
    {
        PositionLast -= v * dt;
    }

    glm::vec2 getVelocity(float dt) const
    {
        return (Position - PositionLast) / dt;
    }
};

class VerletIntegration
{
public:
	VerletIntegration();

	VerletObject& AddObject(glm::vec2 position, float radius)
	{
		return m_Objects.emplace_back(position, radius);
	}

	void Step(glm::vec2 viewportOffset, glm::vec2 viewportSize, float dt);
	void Render(glm::vec2 viewportOffset, glm::vec2 viewportSize);
private:
    void ApplyGravity();
    void CheckCollisions(float dt);
    void ApplyConstraint();
    void UpdateObjects(float dt);
private:
    uint32_t m_SubSteps = 1;
	glm::vec2 m_Gravity = { 0.0f, 1000.0f };
    glm::vec2 m_ConstraintCenter;
    float m_ConstraintRadius = 300.0f;
	std::vector<VerletObject> m_Objects;
    float m_Time = 0.0f;
    float m_FrameDt = 0.0f;
};

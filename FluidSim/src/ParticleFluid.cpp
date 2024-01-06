#include "stpch.h"
#include "ParticleFluid.h"

#include <Stengine.h>
#include <cmath>
#include <limits>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#define PI 3.14159265358979f
#define MASS 1.0f
#define WIDTH 1920
#define HEIGHT 1080

static int2 s_CellOffsets[] = {
	{ -1,  1 },
	{  0,  1 },
	{  1,  1 },
	{ -1,  0 },
	{  0,  0 },
	{  1,  0 },
	{ -1, -1 },
	{  0, -1 },
	{  1, -1 }
};

glm::vec4 MapSpeedToColor(const glm::vec2& velocity, float maxSpeed) 
{
	// Define colors for different speed ranges
	glm::vec3 slowColor =		{ 0.0f, 0.0f, 1.0f }; // Blue
	glm::vec3 mediumColor =		{ 0.0f, 1.0f, 1.0f }; // Cyan
	glm::vec3 moderateColor =	{ 0.0f, 0.5f, 1.0f }; // Teal
	glm::vec3 fastColor =		{ 1.0f, 1.0f, 0.0f }; // Yellow
	glm::vec3 fasterColor =		{ 1.0f, 0.5f, 0.0f }; // Orange
	glm::vec3 maxColor =		{ 1.0f, 0.0f, 0.0f }; // Red

	// Normalize speed to [0, 1] range
	float speedNormalized = glm::length(velocity) / maxSpeed;

	// Use glm::mix to interpolate between colors based on speed
	glm::vec3 resultColor;

	if		(speedNormalized < 0.2f)	resultColor = glm::mix(slowColor,		mediumColor,	 speedNormalized * 5.0f);
	else if (speedNormalized < 0.4f)	resultColor = glm::mix(mediumColor,		moderateColor,	(speedNormalized - 0.2f) * 5.0f);
	else if (speedNormalized < 0.6f)	resultColor = glm::mix(moderateColor,	fastColor,		(speedNormalized - 0.4f) * 5.0f);
	else if (speedNormalized < 0.8f)	resultColor = glm::mix(fastColor,		fasterColor,	(speedNormalized - 0.6f) * 5.0f);
	else								resultColor = glm::mix(fasterColor,		maxColor,		(speedNormalized - 0.8f) * 5.0f);

	// Return the resulting color with alpha set to 1.0
	return { resultColor, 1.0f };
}


ParticleFluid::ParticleFluid()
{
	GenerateParticleGrid(80, 80, 10.0f);
}

void ParticleFluid::GenerateParticleGrid(int width, int height, float spacing)
{
	glm::vec2 offset = glm::vec2{ width * spacing - spacing, height * spacing - spacing } / 2.0f;
	for (int i = 0; i < width; ++i)
	{
		for (int j = 0; j < height; ++j)
		{
			Particle particle;
			particle.Position = glm::vec2{ i * spacing, j * spacing } - offset;
			particle.Velocity = { 0, 0 };
			particle.Color = { 1, 1, 1, 1 };
			m_Particles.push_back(particle);
		}
	}
}

void ParticleFluid::Step(glm::vec2 viewportOffset, glm::vec2 viewportSize, float dt)
{
	ST_PROFILE_FUNCTION();
	dt *= 2.0f;
	float staticDeltaTime = 1.0f / 120.0f;

	glm::vec2 pos = Sten::Input::GetMousePosition() - viewportOffset - viewportSize / 2.0f;
	pos.y = viewportSize.y - pos.y - viewportSize.y;

	m_PredictedPositions.resize(m_Particles.size());
	for (int i = 0; i < m_Particles.size(); i++)
	{
		// External forces
		m_Particles[i].Velocity += CalculateExternalForces(m_Particles[i], pos, m_InteractionRadius,
			Sten::Input::IsMouseButtonPressed(Sten::Mouse::ButtonLeft) ? m_InteractionStrength :
			(Sten::Input::IsMouseButtonPressed(Sten::Mouse::ButtonRight) ? -m_InteractionStrength : 0.0f)) * staticDeltaTime;

		m_PredictedPositions[i] = m_Particles[i].Position + m_Particles[i].Velocity * staticDeltaTime;
	}

	UpdateSpatialLookup(m_PredictedPositions);

	m_Densities.resize(m_Particles.size());
	for (int i = 0; i < m_Particles.size(); i++)
	{
		m_Densities[i] = CalculateDensity(m_PredictedPositions[i]);
		m_Particles[i].Velocity += CalculateViscosityForce(i) * staticDeltaTime;
	}

	for (int i = 0; i < m_Particles.size(); i++)
	{
		glm::vec2 pressureForce = -CalculatePressureForce(i);
		glm::vec2 acceleration = pressureForce / m_Densities[i];
		m_Particles[i].Velocity += acceleration * staticDeltaTime;
	}

	for (int i = 0; i < m_Particles.size(); i++)
	{
		m_Particles[i].Position += m_Particles[i].Velocity * dt;
		if (std::abs(m_Particles[i].Position.x) > WIDTH / 2.0f)
		{
			m_Particles[i].Position.x = WIDTH / 2.0f * (m_Particles[i].Position.x / std::abs(m_Particles[i].Position.x));
			m_Particles[i].Velocity.x *= -m_CollisionDamping;
		}
		if (std::abs(m_Particles[i].Position.y) > HEIGHT / 2.0f)
		{
			m_Particles[i].Position.y = HEIGHT / 2.0f * (m_Particles[i].Position.y / std::abs(m_Particles[i].Position.y));
			m_Particles[i].Velocity.y *= -m_CollisionDamping;
		}
		m_Particles[i].Color = MapSpeedToColor(m_Particles[i].Velocity, 100.0f);
	}
}

void ParticleFluid::Render()
{
	ST_PROFILE_FUNCTION();

	for (int i = 0; i < m_Particles.size(); i++)
	{
		Particle particle = m_Particles[i];
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), { particle.Position.x, particle.Position.y, 0.0f })
			* glm::scale(glm::mat4(1.0f), { 5.0f, 5.0f, 1.0f });
		Sten::Renderer2D::DrawCircle({ transform, particle.Color });
	}

	//for (int i = -WIDTH; i < WIDTH; i++)
	//for (int j = -HEIGHT; j < HEIGHT; j++)
	//{
	//	float d = CalculateDensity({i, j}) * 300.0f;
	//	Sten::Renderer2D::DrawQuad({ i, j, -0.1f }, { 1, 1 }, {d / 7.0f, d / 10.0f, d, 1.0f});
	//}
}

void ParticleFluid::OnImGuiRender()
{
	ST_PROFILE_FUNCTION();

	ImGui::Begin("Particle Simulation");

	ImGui::DragFloat("Smoothing Radius", &m_SmoothingRadius, 0.1f, 0.0f, 1000.0f);
	ImGui::DragFloat("Target Density", &m_TargetDensity, 0.1f, 0.0f, 1000.0f);
	ImGui::DragFloat("Pressure Multiplier", &m_PressureMultiplier, 0.1f, 0.0f, 1000.0f);
	ImGui::DragFloat("Gravity", &m_Gravity, 0.1f, -1000.0f, 1000.0f);
	ImGui::DragFloat("Collision Damping", &m_CollisionDamping, 0.05f, 0.0f, 1.0f);
	ImGui::DragFloat("Viscosity Strengh", &m_ViscosityStrength, 0.1f, 0.0f, 1000.0f);

	ImGui::DragFloat("Interaction Strengh", &m_InteractionStrength, 1.0f, 0.0f, 10000.0f);
	ImGui::DragFloat("Interaction Radius", &m_InteractionRadius, 1.0f, 0.0f, 1000.0f);

	ImGui::End();
}

float ParticleFluid::SmoothingFunction(float dst)
{
	if (dst >= m_SmoothingRadius) return 0.0f;

	float volume = (PI * std::pow(m_SmoothingRadius, 4)) / 6.0f;
	return (m_SmoothingRadius - dst) * (m_SmoothingRadius - dst) / volume;
}

float ParticleFluid::SmoothingFunctionDerivative(float dst)
{
	if (dst >= m_SmoothingRadius) return 0.0f;

	float scale = 12.0f / (std::pow(m_SmoothingRadius, 4) * PI);
	return (dst - m_SmoothingRadius) * scale;
}

float ParticleFluid::ViscositySmoothingFunction(float dst)
{
	float volume = PI * std::pow(m_SmoothingRadius, 8) / 4.0f;
	float value = std::max(0.0f, m_SmoothingRadius * m_SmoothingRadius - dst * dst);
	return value * value * value / volume;
}

float ParticleFluid::CalculateDensity(glm::vec2 samplePoint)
{
	float density = 0.0f;

	int2 center = PositionToCellCoord(samplePoint);
	float sqrRadius = m_SmoothingRadius * m_SmoothingRadius;

	for (int i = 0; i < 9; i++)
	{
		int2 offset = s_CellOffsets[i];
		unsigned int key = GetKeyFromHash(HashCell(center + offset));
		int cellStartIndex = m_StartIndices[key];

		for (int j = cellStartIndex; j < m_SpatialLookup.size(); j++)
		{
			if (m_SpatialLookup[j].key != key) break;

			int particleIndex = m_SpatialLookup[j].index;
			glm::vec2 off = m_PredictedPositions[particleIndex] - samplePoint;
			float sqrDst = off.x * off.x + off.y * off.y;

			if (sqrDst <= sqrRadius)
			{
				float dst = glm::sqrt(sqrDst);
				float influence = SmoothingFunction(dst);
				density += MASS * influence;
			}
		}
	}

	return density;
}

glm::vec2 ParticleFluid::CalculatePressureForce(int targetIndex)
{
	ST_PROFILE_FUNCTION();

	glm::vec2 pressureForce = { 0, 0 };

	int2 center = PositionToCellCoord(m_PredictedPositions[targetIndex]);
	float sqrRadius = m_SmoothingRadius * m_SmoothingRadius;

	for (int i = 0; i < 9; i++)
	{
		int2 offset = s_CellOffsets[i];
		unsigned int key = GetKeyFromHash(HashCell(center + offset));
		int cellStartIndex = m_StartIndices[key];

		for (int j = cellStartIndex; j < m_SpatialLookup.size(); j++)
		{
			if (m_SpatialLookup[j].key != key) break;

			int particleIndex = m_SpatialLookup[j].index;
			glm::vec2 off = m_PredictedPositions[particleIndex] - m_PredictedPositions[targetIndex];
			float sqrDst = off.x * off.x + off.y * off.y;

			if (sqrDst <= sqrRadius)
			{
				if (particleIndex == targetIndex) continue;

				float dst = glm::sqrt(sqrDst);
				glm::vec2 dir = dst == 0 ? glm::vec2{ 1.0f, 1.0f } / glm::length(glm::vec2{ 1.0f, 1.0f }) : (off / dst);

				float slope = SmoothingFunctionDerivative(dst);
				float density = m_Densities[particleIndex];
				float sharedPressure = CalculateSharedPressure(density, m_Densities[targetIndex]);

				pressureForce += sharedPressure * dir * slope * MASS / density;
			}
		}
	}

	return pressureForce;
}

glm::vec2 ParticleFluid::CalculateExternalForces(const Particle& particle, glm::vec2 inputPos, float radius, float strength)
{
	// Gravity
	glm::vec2 gravityAccel = { 0, m_Gravity };

	// Input interactions modify gravity
	if (strength != 0) {
		glm::vec2 inputPointOffset = inputPos - particle.Position;
		float sqrDst = dot(inputPointOffset, inputPointOffset);
		if (sqrDst < radius * radius)
		{
			float dst = sqrt(sqrDst);
			float edgeT = (dst / radius);
			float centreT = 1.0f - edgeT;
			glm::vec2 dirToCentre = inputPointOffset / dst;

			float gravityWeight = 1.0f - (centreT * std::min(1.0f, std::max(0.0f, strength / 10.0f)));
			glm::vec2 accel = gravityAccel * gravityWeight + dirToCentre * centreT * strength;
			accel -= particle.Velocity * centreT;
			return accel;
		}
	}

	return gravityAccel;
}

glm::vec2 ParticleFluid::CalculateViscosityForce(int targetIndex)
{
	glm::vec2 viscosityForce = { 0, 0 };

	int2 center = PositionToCellCoord(m_PredictedPositions[targetIndex]);
	float sqrRadius = m_SmoothingRadius * m_SmoothingRadius;

	for (int i = 0; i < 9; i++)
	{
		int2 offset = s_CellOffsets[i];
		unsigned int key = GetKeyFromHash(HashCell(center + offset));
		int cellStartIndex = m_StartIndices[key];

		for (int j = cellStartIndex; j < m_SpatialLookup.size(); j++)
		{
			if (m_SpatialLookup[j].key != key) break;

			int particleIndex = m_SpatialLookup[j].index;
			glm::vec2 off = m_PredictedPositions[particleIndex] - m_PredictedPositions[targetIndex];
			float sqrDst = off.x * off.x + off.y * off.y;

			if (sqrDst <= sqrRadius)
			{
				float dst = glm::sqrt(sqrDst);
				glm::vec2 neighbourVelocity = m_Particles[particleIndex].Velocity;
				viscosityForce += (neighbourVelocity - m_Particles[targetIndex].Velocity) * ViscositySmoothingFunction(dst);
			}
		}
	}

	return viscosityForce;
}

float ParticleFluid::ConvertDensityToPressure(float density)
{
	float densityError = density - m_TargetDensity;
	float pressure = densityError * m_PressureMultiplier;
	return pressure;
}

float ParticleFluid::CalculateSharedPressure(float d1, float d2)
{
	float pressureA = ConvertDensityToPressure(d1);
	float pressureB = ConvertDensityToPressure(d2);
	return (pressureA + pressureB) / 2.0f;
}

bool CompareByKey(const Entry& entry1, const Entry& entry2)
{
	return entry1.key < entry2.key;
}

void ParticleFluid::UpdateSpatialLookup(const std::vector<glm::vec2>& positions)
{
	m_SpatialLookup.resize(positions.size());
	m_StartIndices.resize(positions.size());

	for (int i = 0; i < positions.size(); i++)
	{
		int2 cell = PositionToCellCoord(positions[i]);
		unsigned int cellKey = GetKeyFromHash(HashCell(cell));
		m_SpatialLookup[i] = { i, cellKey };
		m_StartIndices[i] = 4294967295;
	}

	// sort spatial lookup
	std::sort(m_SpatialLookup.begin(), m_SpatialLookup.end(), CompareByKey);

	for (long i = 0; i < positions.size(); i++)
	{
		unsigned int key = m_SpatialLookup[i].key;
		unsigned int keyPrev = i == 0 ? 4294967295 : m_SpatialLookup[i - 1].key;
		if (key != keyPrev)
		{
			m_StartIndices[key] = i;
		}
	}
}

int2 ParticleFluid::PositionToCellCoord(glm::vec2 point)
{
	int cellX = point.x / m_SmoothingRadius;
	int cellY = point.y / m_SmoothingRadius;
	return { cellX, cellY };
}

unsigned int ParticleFluid::HashCell(int2 cell)
{
	unsigned int a = (unsigned int)cell.x * 15823;
	unsigned int b = (unsigned int)cell.y * 9737333;
	return a + b;
}

unsigned int ParticleFluid::GetKeyFromHash(unsigned int hash)
{
	return hash % (unsigned int)m_SpatialLookup.size();
}

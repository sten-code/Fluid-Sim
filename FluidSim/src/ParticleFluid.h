#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Particle
{
	glm::vec2 Position;
	glm::vec2 Velocity;
	glm::vec4 Color;
};

struct Entry
{
	int index;
	unsigned int key;
};

typedef glm::vec<2, int> int2;

class ParticleFluid
{
public:
	ParticleFluid();
	virtual ~ParticleFluid() = default;

	void GenerateParticleGrid(int rows, int cols, float spacing);

	void Step(glm::vec2 viewportOffset, glm::vec2 viewportSize, float dt);
	void Render();
	void OnImGuiRender();
private:
	// Smoothing Functions
	float SmoothingFunction(float dst);
	float SmoothingFunctionDerivative(float dst);
	float ViscositySmoothingFunction(float dst);

	// Forces
	glm::vec2 CalculatePressureForce(int particleIndex);
	glm::vec2 CalculateExternalForces(const Particle& particle, glm::vec2 inputPos, float radius, float strength);
	glm::vec2 CalculateViscosityForce(int targetIndex);

	// Pressure calculations
	float CalculateDensity(glm::vec2 samplePoint);
	float ConvertDensityToPressure(float density);
	float CalculateSharedPressure(float d1, float d2);
	
	// Spatial Lookup
	void UpdateSpatialLookup(const std::vector<glm::vec2>& positions);
	int2 PositionToCellCoord(glm::vec2 point);
	unsigned int HashCell(int2 cell);
	unsigned int GetKeyFromHash(unsigned int hash);
private:
	std::vector<Particle> m_Particles;
	std::vector<float> m_Densities;
	std::vector<Entry> m_SpatialLookup;
	std::vector<unsigned int> m_StartIndices;
	std::vector<glm::vec2> m_PredictedPositions;
	float m_SmoothingRadius = 11.0f;
	float m_TargetDensity = 20.0f;
	float m_PressureMultiplier = 200.0f;
	float m_Gravity = -100.0f;
	float m_CollisionDamping = 0.9f;
	float m_ViscosityStrength = 1.0f;
	float m_InteractionStrength = 300.0f;
	float m_InteractionRadius = 300.0f;

	//float m_SmoothingRadius = 30.0f;
	//float m_TargetDensity = 2.0f;
	//float m_PressureMultiplier = 250.0f;
	//float m_Gravity = 0.0f;
	//float m_CollisionDamping = 0.9f;
	//float m_ViscosityStrength = 1.0f;
	//float m_InteractionStrength = 300.0f;
	//float m_InteractionRadius = 300.0f;
};
#pragma once

#include <Stengine.h>

class NavierStokesFluid
{
public:
	NavierStokesFluid(int width, int height, int scale, float diffusion, float viscosity, int iterations);
	virtual ~NavierStokesFluid() = default;

    void Resize(int width, int height);

    void AddDensity(int x, int y, int radius, float amount);
    void AddVelocity(int x, int y, int radius, float amountX, float amountY);

    void Diffuse(int b, float* x, float* x0, float diff, float dt);
    void Project(float* velocX, float* velocY, float* p, float* div);
    void Advect(int b, float* d, float* d0, float* velocX, float* velocY, float dt);

    void Step(float dt);
    void Render();

    void LinearSolve(int b, float* x, float* x0, float a, float c);
    void SetBound(int b, float* x);

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    int GetScale() const { return m_Scale; }
private:
    int m_Width, m_Height;
    int m_Scale;
    int m_Iterations;
    float m_Diffusion;
    float m_Viscosity;

    float* m_PreviousDensity;
    float* m_Density;

    float* m_VelocityX;
    float* m_VelocityY;

    float* m_PreviousVelocityX;
    float* m_PreviousVelocityY;
};
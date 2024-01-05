#include "NavierStokesFluid.h"

#include <Stengine.h>
#include <cmath>

#define IX(x, y) (std::clamp(x, 0, m_Width - 1) + std::clamp(y, 0, m_Height - 1) * m_Width)

NavierStokesFluid::NavierStokesFluid(int width, int height, int scale, float diffusion, float viscosity, int iterations)
	: m_Width(width), m_Height(height), m_Scale(scale), m_Diffusion(diffusion), m_Viscosity(viscosity), m_Iterations(iterations)
{
    ST_PROFILE_FUNCTION();

    int c = width * height;
    m_PreviousDensity = new float[c];
    m_Density = new float[c];

    m_VelocityX = new float[c];
    m_VelocityY = new float[c];

    m_PreviousVelocityX = new float[c];
    m_PreviousVelocityY = new float[c];

    memset(m_PreviousDensity, 0, c * sizeof(float));
    memset(m_Density, 0, c * sizeof(float));
    memset(m_VelocityX, 0, c * sizeof(float));
    memset(m_VelocityY, 0, c * sizeof(float));
    memset(m_PreviousVelocityX, 0, c * sizeof(float));
    memset(m_PreviousVelocityY, 0, c * sizeof(float));
}

void NavierStokesFluid::Resize(int width, int height)
{
    m_Width = width;
    m_Height = height;

    delete[] m_PreviousDensity;
    delete[] m_Density;
    delete[] m_VelocityX;
    delete[] m_VelocityY;
    delete[] m_PreviousVelocityX;
    delete[] m_PreviousVelocityY;

    int c = width * height;
    m_PreviousDensity = new float[c];
    m_Density = new float[c];

    m_VelocityX = new float[c];
    m_VelocityY = new float[c];

    m_PreviousVelocityX = new float[c];
    m_PreviousVelocityY = new float[c];

    memset(m_PreviousDensity, 0, c * sizeof(float));
    memset(m_Density, 0, c * sizeof(float));
    memset(m_VelocityX, 0, c * sizeof(float));
    memset(m_VelocityY, 0, c * sizeof(float));
    memset(m_PreviousVelocityX, 0, c * sizeof(float));
    memset(m_PreviousVelocityY, 0, c * sizeof(float));
}

void NavierStokesFluid::AddDensity(int x, int y, int radius, float amount)
{
    ST_PROFILE_FUNCTION();

    int r2 = radius * radius;

    for (int i = x - radius; i <= x + radius; i++) {
        for (int j = y - radius; j <= y + radius; j++) {
            // Calculate the distance from the specified point (centerX, centerY)
            float distance = (i - x) * (i - x) + (j - y) * (j - y);

            // Check if the distance is within the specified radius
            if (distance <= r2) {
                // Set the value of the element to the specified value
                m_Density[IX(i, j)] += amount;
            }
        }
    }
}

void NavierStokesFluid::AddVelocity(int x, int y, int radius, float amountX, float amountY)
{
    ST_PROFILE_FUNCTION();

    int r2 = radius * radius;

    for (int i = x - radius; i <= x + radius; i++) {
        for (int j = y - radius; j <= y + radius; j++) {
            // Calculate the distance from the specified point (centerX, centerY)
            float distance = (i - x) * (i - x) + (j - y) * (j - y);

            // Check if the distance is within the specified radius
            if (distance <= r2) {
                // Set the value of the element to the specified value    
                int index = IX(i, j);
                m_VelocityX[index] += amountX;
                m_VelocityY[index] += amountY;
            }
        }
    }
}

void NavierStokesFluid::Diffuse(int b, float* x, float* x0, float diff, float dt)
{
    ST_PROFILE_FUNCTION();

    float a = dt * diff * (m_Width - 2) * (m_Height * 2);
    LinearSolve(b, x, x0, a, 1 + a * 6);
}

void NavierStokesFluid::LinearSolve(int b, float* x, float* x0, float a, float c)
{
    ST_PROFILE_FUNCTION();

    float cRecip = 1.0 / c;
    for (int k = 0; k < m_Iterations; k++)
    {
        for (int j = 1; j < m_Height - 1; j++)
        {
            for (int i = 1; i < m_Width - 1; i++)
            {
                x[IX(i, j)] =
                    (x0[IX(i, j)] + a *
                        (x[IX(i + 1, j)]
                            + x[IX(i - 1, j)]
                            + x[IX(i, j + 1)]
                            + x[IX(i, j - 1)]
                            )) * cRecip;
            }
        }
        SetBound(b, x);
    }
}

void NavierStokesFluid::SetBound(int b, float* x)
{
    ST_PROFILE_FUNCTION();

    for (int i = 1; i < m_Width - 1; i++) {
        x[IX(i, 0           )] = b == 2 ? -x[IX(i, 1           )] : x[IX(i, 1           )];
        x[IX(i, m_Height - 1)] = b == 2 ? -x[IX(i, m_Height - 2)] : x[IX(i, m_Height - 2)];
    }

    for (int j = 1; j < m_Height - 1; j++) {
        x[IX(0, j          )] = b == 1 ? -x[IX(1, j          )] : x[IX(1, j         )];
        x[IX(m_Width - 1, j)] = b == 1 ? -x[IX(m_Width - 2, j)] : x[IX(m_Width - 2, j)];
    }

    x[IX(0, 0)] = 0.5 * (x[IX(1, 0)]
                       + x[IX(0, 1)]);

    x[IX(0, m_Height - 1)] = 0.5 * (x[IX(1, m_Height - 1)]
                                  + x[IX(0, m_Height - 2)]);

    x[IX(m_Width - 1, 0)] = 0.5 * (x[IX(m_Width - 2, 0)]
                                 + x[IX(m_Width - 1, 1)]);

    x[IX(m_Width - 1, m_Height - 1)] = 0.5 * (x[IX(m_Width - 2, m_Height - 1)] +
                                              x[IX(m_Width - 1, m_Height - 2)]);
}

void NavierStokesFluid::Project(float* velocX, float* velocY, float* p, float* div)
{
    ST_PROFILE_FUNCTION();

    for (int j = 1; j < m_Height - 1; j++) {
        for (int i = 1; i < m_Width - 1; i++) {
            div[IX(i, j)] = -0.5f * (
                velocX[IX(i + 1, j)]
                - velocX[IX(i - 1, j)]
                + velocY[IX(i, j + 1)]
                - velocY[IX(i, j - 1)]
                ) / ((m_Width + m_Height) / 2.0f);
            p[IX(i, j)] = 0;
        }
    }
    SetBound(0, div);
    SetBound(0, p);
    LinearSolve(0, p, div, 1, 6);

    for (int j = 1; j < m_Height - 1; j++) {
        for (int i = 1; i < m_Width - 1; i++) {
            velocX[IX(i, j)] -= 0.5f * (p[IX(i + 1, j)]
                - p[IX(i - 1, j)]) * m_Width;
            velocY[IX(i, j)] -= 0.5f * (p[IX(i, j + 1)]
                - p[IX(i, j - 1)]) * m_Height;
        }
    }
    SetBound(1, velocX);
    SetBound(2, velocY);
}

void NavierStokesFluid::Advect(int b, float* d, float* d0, float* velocX, float* velocY, float dt)
{
    ST_PROFILE_FUNCTION();

    float i0, i1, j0, j1;

    float dtx = dt * (m_Width - 2);
    float dty = dt * (m_Height - 2);

    float s0, s1, t0, t1;
    float tmp1, tmp2, x, y;

    float ifloat, jfloat;
    int i, j;

    for (j = 1, jfloat = 1; j < m_Height - 1; j++, jfloat++) {
        for (i = 1, ifloat = 1; i < m_Width - 1; i++, ifloat++) {
            tmp1 = dtx * velocX[IX(i, j)];
            tmp2 = dty * velocY[IX(i, j)];
            x = ifloat - tmp1;
            y = jfloat - tmp2;

            if (x < 0.5f) x = 0.5f;
            if (x > m_Width + 0.5f) x = m_Width + 0.5f;
            i0 = std::floor(x);
            i1 = i0 + 1.0f;
            if (y < 0.5f) y = 0.5f;
            if (y > m_Height + 0.5f) y = m_Height + 0.5f;
            j0 = std::floor(y);
            j1 = j0 + 1.0f;

            s1 = x - i0;
            s0 = 1.0f - s1;
            t1 = y - j0;
            t0 = 1.0f - t1;

            int i0i = i0;
            int i1i = i1;
            int j0i = j0;
            int j1i = j1;

            d[IX(i, j)] =
                s0 * (t0 * d0[IX(i0i, j0i)] + t1 * d0[IX(i0i, j1i)]) +
                s1 * (t0 * d0[IX(i1i, j0i)] + t1 * d0[IX(i1i, j1i)]);
        }
    }
    SetBound(b, d);
}

void NavierStokesFluid::Step(float dt)
{
    //for (int y = 0; y < m_Height; y++)
    //{
    //    int index = IX(5, y);
    //    m_VelocityX[index] += 1.0f;
    //}

    Diffuse(1, m_PreviousVelocityX, m_VelocityX, m_Viscosity, dt);
    Diffuse(2, m_PreviousVelocityY, m_VelocityY, m_Viscosity, dt);

    Project(m_PreviousVelocityX, m_PreviousVelocityY, m_VelocityX, m_VelocityY);

    Advect(1, m_VelocityX, m_PreviousVelocityX, m_PreviousVelocityX, m_PreviousVelocityY, dt);
    Advect(2, m_VelocityY, m_PreviousVelocityY, m_PreviousVelocityX, m_PreviousVelocityY, dt);

    Project(m_VelocityX, m_VelocityY, m_PreviousVelocityX, m_PreviousVelocityY);

    Diffuse(0, m_PreviousDensity, m_Density, m_Diffusion, dt);
    Advect(0, m_Density, m_PreviousDensity, m_VelocityX, m_VelocityY, dt);
}

void NavierStokesFluid::Render()
{
    for (int i = 0; i < m_Width; i++)
    {
        for (int j = 0; j < m_Height; j++)
        {
            float x = (i - m_Width / 2.0f) * m_Scale;
            float y = (j - m_Height / 2.0f) * m_Scale;
            float d = m_Density[IX(i, j)];
            float vx = std::abs(m_VelocityX[IX(i, j)]);
            float vy = std::abs(m_VelocityY[IX(i, j)]);
            float avg = (vx + vy) / 2.0f;
            Sten::Renderer2D::DrawQuad({ x, y }, { m_Scale, m_Scale }, {avg / 3.0f + 0.2f, avg / 2.0f + 0.3f, 0.8f, 1.0f});
        }
    }
}

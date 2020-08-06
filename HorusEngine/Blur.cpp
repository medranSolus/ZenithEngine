#include "Blur.h"
#include "Math.h"

namespace GFX::Pipeline
{
	void Blur::SetKernel(Graphics& gfx, int radius, float sigma)
	{
		assert(radius < maxRadius);
		this->radius = radius;
		this->sigma = sigma;
		GaussBuffer buffer;
		buffer.radius = radius;
		float sum = 0.0f;
		for (int i = 0; i <= radius; ++i)
		{
			const float g = gauss(static_cast<float>(i), sigma);
			sum += g;
			buffer.coefficients[i].x = g;
		}
		for (int i = 0; i <= radius; ++i)
			buffer.coefficients[i].x /= sum;
		kernelBuffer.Update(gfx, buffer);
	}

	void Blur::Bind(Graphics& gfx) noexcept
	{
		shader.Bind(gfx);
		kernelBuffer.Bind(gfx);
		controlBuffer.Bind(gfx);
	}

	void Blur::ShowWindow(Graphics& gfx) noexcept
	{
		int r = radius;
		float s = sigma;
		ImGui::SliderInt("Blur radius", &r, 1, maxRadius - 1);
		ImGui::SliderFloat("Blur sigma", &s, 0.0f, 10.0f, "%.1f");
		if (r != radius || notEquals(s, sigma))
			SetKernel(gfx, r, s);
	}
}
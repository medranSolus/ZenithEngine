#include "PointLight.h"
#include "Math.h"

namespace GFX::Light
{
	PointLight::PointLight(Graphics & gfx, float x0, float y0, float z0, float radius)
		: mesh(gfx, { 1.0f,1.0f,1.0f }, x0, y0, z0, 3, 3, radius, radius, radius), buffer(gfx)
	{
		lightBuffer =
		{
			{ 0.05f, 0.05f, 0.05f, 1.0f },
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			mesh.GetPos(),
			1.0f,
			1.0f,
			0.045f,
			0.0075
		};
	}

	void PointLight::Bind(Graphics & gfx, const Camera & camera) const noexcept
	{
		DirectX::XMStoreFloat3(&lightBuffer.pos, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&mesh.GetPos()), camera.GetView()));
		buffer.Update(gfx, lightBuffer);
		buffer.Bind(gfx);
	}
}

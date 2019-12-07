#pragma once
#include "Globe.h"
#include "ConstantPixelBuffer.h"

namespace GFX::Light
{
	class PointLight
	{
		struct LightConstantBuffer
		{
			DirectX::XMFLOAT3 pos;
			float padding = 0.0f;
		};

		mutable Object::Globe mesh;
		mutable Resource::ConstantPixelBuffer<LightConstantBuffer> buffer;

	public:
		PointLight(Graphics & gfx, float x0, float y0, float z0, float radius = 0.5f)
			: mesh(gfx, x0, y0, z0, 3, 3, radius, radius, radius), buffer(gfx) {}

		inline void Move(float dX, float dY, float dZ) noexcept { mesh.Update(dX, dY, dZ); }
		inline void SetPos(float x, float y, float z) noexcept { mesh.SetPos({ x, y, z }); }
		inline void Draw(Graphics & gfx) const noexcept(!IS_DEBUG) { mesh.Draw(gfx); }

		void Bind(Graphics & gfx) const noexcept;
	};
}

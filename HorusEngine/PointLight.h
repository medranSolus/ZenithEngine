#pragma once
#include "SolidGlobe.h"
#include "ConstantPixelBuffer.h"
#include "ShaderConstantBuffers.h"
#include "Camera.h"

namespace GFX::Light
{
	class PointLight
	{
		mutable Resource::LightConstantBuffer lightBuffer;
		mutable Object::SolidGlobe mesh;
		mutable Resource::ConstantPixelBuffer<Resource::LightConstantBuffer> buffer;

	public:
		PointLight(Graphics & gfx, float x0, float y0, float z0, float radius = 0.5f);

		inline void Move(float dX, float dY, float dZ) noexcept { mesh.Update(dX, dY, dZ); }
		inline void SetPos(float x, float y, float z) noexcept { mesh.SetPos({ x, y, z }); }
		inline void Draw(Graphics & gfx) const noexcept(!IS_DEBUG) { mesh.Draw(gfx); }

		void Bind(Graphics & gfx, const Camera & camera) const noexcept;
	};
}

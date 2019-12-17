#pragma once
#include "ObjectBase.h"

namespace GFX::Object
{
	class Model : public ObjectBase<Model>
	{
	public:
		Model(Graphics & gfx, const std::string & filename, float x0, float y0, float z0, float scale = 1.0f);

		void Update(float dX, float dY, float dZ, float angleDZ = 0.0f, float angleDX = 0.0f, float angleDY = 0.0f) noexcept override;
		DirectX::XMMATRIX GetTransformMatrix() const noexcept override;
	};
}

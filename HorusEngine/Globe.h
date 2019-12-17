#pragma once
#include "ObjectBase.h"

namespace GFX::Object
{
	class Globe : public ObjectBase<Globe>
	{
		DirectX::XMFLOAT3 size;

	public:
		Globe(Graphics & gfx, BasicType::ColorFloat material, float x0, float y0, float z0, unsigned int latitudeDensity, unsigned int longitudeDensity,
			float height = 1.0f, float width = 1.0f, float length = 1.0f);

		void Update(float dX, float dY, float dZ, float angleDZ = 0.0f, float angleDX = 0.0f, float angleDY = 0.0f) noexcept override;
		DirectX::XMMATRIX GetTransformMatrix() const noexcept override;
	};
}

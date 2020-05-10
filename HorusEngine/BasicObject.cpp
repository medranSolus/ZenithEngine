#include "BasicObject.h"

namespace GFX
{
	void BasicObject::Update(const DirectX::XMFLOAT3& delta, const DirectX::XMFLOAT3& deltaAngle) noexcept
	{
		DirectX::XMStoreFloat3(&pos, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&delta)));
		DirectX::XMStoreFloat3(&angle,
			DirectX::XMVectorModAngles(DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&angle),
				DirectX::XMLoadFloat3(&deltaAngle))));
	}
}
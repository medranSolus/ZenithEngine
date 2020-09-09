#include "Math.h"

namespace Math
{
	DirectX::XMFLOAT3 Add(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2) noexcept
	{
		DirectX::XMFLOAT3 out;
		DirectX::XMStoreFloat3(&out, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&v1), DirectX::XMLoadFloat3(&v2)));
		return std::move(out);
	}

	DirectX::XMFLOAT3 AddNormal(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2) noexcept
	{
		DirectX::XMFLOAT3 out;
		DirectX::XMStoreFloat3(&out, DirectX::XMVector3Normalize(DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&v1), DirectX::XMLoadFloat3(&v2))));
		return std::move(out);
	}
}
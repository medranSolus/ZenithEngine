#include "Math.h"

DirectX::XMFLOAT3 add(const DirectX::XMFLOAT3 & v1, const DirectX::XMFLOAT3 & v2)
{
	DirectX::XMFLOAT3 out;
	DirectX::XMStoreFloat3(&out, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&v1), DirectX::XMLoadFloat3(&v2)));
	return std::move(out);
}

DirectX::XMFLOAT3 addNormal(const DirectX::XMFLOAT3 & v1, const DirectX::XMFLOAT3 & v2)
{
	DirectX::XMFLOAT3 out;
	DirectX::XMStoreFloat3(&out, DirectX::XMVector3Normalize(DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&v1), DirectX::XMLoadFloat3(&v2))));
	return std::move(out);
}
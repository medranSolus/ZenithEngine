#include "IBufferResource.h"

namespace GFX::Pipeline::Resource
{
	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> IBufferResource::nullShaderResource = nullptr;
}
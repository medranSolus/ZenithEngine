#include "IBufferResource.h"

namespace GFX::Pipeline::Resource
{
	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> IBufferResource::nullShaderResource = nullptr;

	IBufferResource::IBufferResource(Graphics& gfx, unsigned int width, unsigned int height) noexcept
	{
		viewport = GFX::Resource::Viewport::Get(gfx, width, height);
	}
}
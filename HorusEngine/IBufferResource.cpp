#include "IBufferResource.h"

namespace GFX::Pipeline::Resource
{
	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> IBufferResource::nullShaderResource = nullptr;

	IBufferResource::IBufferResource(unsigned int width, unsigned int height) noexcept : width(width), height(height)
	{
		// Viewport for different resolutions
		viewport.Width = static_cast<FLOAT>(width);
		viewport.Height = static_cast<FLOAT>(height);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
	}
}
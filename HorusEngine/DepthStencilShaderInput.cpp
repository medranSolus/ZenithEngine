#include "DepthStencilShaderInput.h"
#include "GfxExceptionMacros.h"

namespace GFX::Pipeline::Resource
{
	inline DXGI_FORMAT DepthStencilShaderInput::UsageShaderInput(Usage usage) noexcept
	{
		switch (usage)
		{
		default:
		case Usage::DepthStencil:
			return DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case Usage::ShadowDepth:
			return DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
		}
	}

	DepthStencilShaderInput::DepthStencilShaderInput(Graphics& gfx, unsigned int width, unsigned int height, UINT slot, Usage usage)
		: DepthStencil(gfx, width, height, usage), slot(slot)
	{
		GFX_ENABLE_ALL(gfx);

		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		depthStencilView->GetResource(&resource);

		D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc = {};
		textureViewDesc.Format = UsageShaderInput(usage);
		textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
		textureViewDesc.Texture2D.MipLevels = 1U;
		textureViewDesc.Texture2D.MostDetailedMip = 0U;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateShaderResourceView(resource.Get(), &textureViewDesc, &textureView));
	}
}
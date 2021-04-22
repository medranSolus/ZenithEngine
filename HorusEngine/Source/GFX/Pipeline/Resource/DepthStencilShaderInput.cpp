#include "GFX/Pipeline/Resource/DepthStencilShaderInput.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"

namespace GFX::Pipeline::Resource
{
	constexpr DXGI_FORMAT DepthStencilShaderInput::UsageShaderInput(Usage usage) noexcept
	{
		switch (usage)
		{
		default:
		case Usage::DepthStencil:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case Usage::DepthOnly:
			return DXGI_FORMAT_R32_FLOAT;
		}
	}

	DepthStencilShaderInput::DepthStencilShaderInput(Graphics& gfx, U32 slot, Usage usage)
		: DepthStencilShaderInput(gfx, gfx.GetWidth(), gfx.GetHeight(), slot, usage) {}

	DepthStencilShaderInput::DepthStencilShaderInput(Graphics& gfx, U32 width, U32 height, U32 slot, Usage usage)
		: DepthStencil(gfx, width, height, true, usage), slot(slot)
	{
		GFX_ENABLE_ALL(gfx);

		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		depthStencilView->GetResource(&resource);

		D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc = {};
		textureViewDesc.Format = UsageShaderInput(usage);
		textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
		textureViewDesc.Texture2D.MipLevels = 1;
		textureViewDesc.Texture2D.MostDetailedMip = 0;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateShaderResourceView(resource.Get(), &textureViewDesc, &textureView));
		GFX_SET_ID(textureView.Get(), "DSI" + std::to_string(GetWidth()) + "x" + std::to_string(GetHeight()) +
			"#" + std::to_string(static_cast<bool>(usage)) + "#" + std::to_string(slot));
	}
}
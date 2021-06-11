#include "GFX/Resource/TextureDepthCube.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Pipeline/Resource/RenderTarget.h"
#include "GFX/Graphics.h"

namespace ZE::GFX::Resource
{
	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureDepthCube::nullShaderResource = nullptr;

	TextureDepthCube::TextureDepthCube(Graphics& gfx, U32 size, U32 slot)
		: slot(slot), size(size), stencil(gfx, size)
	{
		assert(slot < D3D11_PS_INPUT_REGISTER_COUNT);
		ZE_GFX_ENABLE_ALL(gfx);

		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.Width = textureDesc.Height = size;
		textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 6;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&textureDesc, nullptr, &texture));

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		viewDesc.Texture2D.MipLevels = 1;
		viewDesc.Texture2D.MostDetailedMip = 0;
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateShaderResourceView(texture.Get(), &viewDesc, &textureView));
		ZE_GFX_SET_RID(textureView.Get());
		depthBuffer = GfxResPtr<Pipeline::Resource::RenderTarget>(gfx, texture, size);
	}

	std::string TextureDepthCube::GenerateRID(U32 size, U32 slot) noexcept
	{
		return "T" + std::to_string(slot) + "#" + std::to_string(size);
	}

	GfxResPtr<TextureDepthCube> TextureDepthCube::Get(Graphics& gfx, U32 size, U32 slot)
	{
		return Codex::Resolve<TextureDepthCube>(gfx, size, slot);
	}
}
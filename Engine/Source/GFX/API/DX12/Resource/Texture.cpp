#include "GFX/API/DX12/Resource/Texture.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12::Resource
{
	Texture::Texture(GFX::Device& dev, GFX::CommandList& cl, const Surface& surface)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx12);

		//cl.Get().dx11.GetContext()->GenerateMips(srv.Get());
		//ZE_GFX_SET_ID(srv, "Texture");
	}

	void Texture::BindPS(GFX::CommandList& cl, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
	}

	void Texture::BindCS(GFX::CommandList& cl, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
	}

	Surface Texture::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return { 1, 1 };
	}
}
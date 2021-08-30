#include "GFX/API/DX12/Resource/TextureCube.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12::Resource
{
	TextureCube::TextureCube(GFX::Device& dev, const std::array<Surface, 6>& surfaces)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx12);

		//ZE_GFX_SET_ID(srv, "TextureCube");
	}

	void TextureCube::BindPS(GFX::CommandList& cl, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
	}

	void TextureCube::BindCS(GFX::CommandList& cl, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
	}

	std::array<Surface, 6> TextureCube::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return
		{
			Surface(1, 1),
			Surface(1, 1),
			Surface(1, 1),
			Surface(1, 1),
			Surface(1, 1),
			Surface(1, 1)
		};
	}
}
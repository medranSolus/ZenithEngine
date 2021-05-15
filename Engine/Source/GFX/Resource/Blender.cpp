#include "GFX/Resource/Blender.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"

namespace ZE::GFX::Resource
{
	Blender::Blender(Graphics& gfx, Type type) : type(type)
	{
		ZE_GFX_ENABLE_ALL(gfx);

		D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT{});
		auto& blendTarget = blendDesc.RenderTarget[0];
		switch (type)
		{
		case Type::Light:
		{
			blendTarget.BlendEnable = TRUE;
			blendTarget.SrcBlend = D3D11_BLEND_ONE;
			blendTarget.DestBlend = D3D11_BLEND_ONE;
			blendTarget.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
			break;
		}
		case Type::Normal:
		{
			blendTarget.BlendEnable = TRUE;
			blendTarget.SrcBlend = D3D11_BLEND_SRC_ALPHA; // Maybe ONE
			blendTarget.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			break;
		}
		}
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateBlendState(&blendDesc, &state));
		ZE_GFX_SET_RID(state.Get());
	}
}
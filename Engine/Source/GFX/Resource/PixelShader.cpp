#include "GFX/Resource/PixelShader.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"
#include <d3dcompiler.h>

namespace ZE::GFX::Resource
{
	PixelShader::PixelShader(Graphics& gfx, const std::string& name) : name(name)
	{
		ZE_GFX_ENABLE_ALL(gfx);
		Microsoft::WRL::ComPtr<ID3DBlob> blob;
		ZE_GFX_THROW_FAILED(D3DReadFileToBlob(Utils::ToUtf8("Shaders\\" + name + ".cso").c_str(), &blob));
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixelShader));
		ZE_GFX_SET_RID(pixelShader.Get());
	}
}
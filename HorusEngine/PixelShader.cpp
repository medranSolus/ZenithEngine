#include "PixelShader.h"
#include "GfxExceptionMacros.h"
#include "Utils.h"
#include <d3dcompiler.h>

namespace GFX::Resource
{
	PixelShader::PixelShader(Graphics& gfx, const std::string& name) : name(name)
	{
		GFX_ENABLE_ALL(gfx);
		Microsoft::WRL::ComPtr<ID3DBlob> blob;
		GFX_THROW_FAILED(D3DReadFileToBlob(Utils::ToUtf8("Shaders\\" + name + ".cso").c_str(), &blob));
		GFX_THROW_FAILED(GetDevice(gfx)->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixelShader));
		SET_DEBUG_NAME_RID(pixelShader.Get());
	}
}
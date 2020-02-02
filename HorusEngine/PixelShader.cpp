#include "PixelShader.h"
#include "Codex.h"
#include "GfxExceptionMacros.h"
#include "Utils.h"
#include <d3dcompiler.h>

namespace GFX::Resource
{
	PixelShader::PixelShader(Graphics& gfx, const std::string& path) : path(path)
	{
		GFX_ENABLE_ALL(gfx);
		Microsoft::WRL::ComPtr<ID3DBlob> blob;
		GFX_THROW_FAILED(D3DReadFileToBlob(toUtf8(path).c_str(), &blob));
		GFX_THROW_FAILED(GetDevice(gfx)->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixelShader));
	}
}
#include "VertexShader.h"
#include "GfxExceptionMacros.h"
#include <d3dcompiler.h>

namespace GFX::Resource
{
	VertexShader::VertexShader(Graphics & gfx, const std::wstring & path)
	{
		GFX_ENABLE_ALL(gfx);
		GFX_THROW_FAILED(D3DReadFileToBlob(path.c_str(), &bytecode));
		GFX_THROW_FAILED(GetDevice(gfx)->CreateVertexShader(bytecode->GetBufferPointer(), 
			bytecode->GetBufferSize(), nullptr, &vertexShader));
	}
}

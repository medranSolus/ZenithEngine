#include "VertexShader.h"
#include "GfxExceptionMacros.h"
#include "Utils.h"
#include <d3dcompiler.h>

namespace GFX::Resource
{
	VertexShader::VertexShader(Graphics& gfx, const std::string& name) : name(name)
	{
		GFX_ENABLE_ALL(gfx);
		GFX_THROW_FAILED(D3DReadFileToBlob(Utils::ToUtf8("Shaders\\" + name + ".cso").c_str(), &bytecode));
		GFX_THROW_FAILED(GetDevice(gfx)->CreateVertexShader(bytecode->GetBufferPointer(),
			bytecode->GetBufferSize(), nullptr, &vertexShader));
		SET_DEBUG_NAME_RID(vertexShader.Get());
	}
}
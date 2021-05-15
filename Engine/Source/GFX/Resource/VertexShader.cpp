#include "GFX/Resource/VertexShader.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"
#include <d3dcompiler.h>

namespace ZE::GFX::Resource
{
	VertexShader::VertexShader(Graphics& gfx, const std::string& name) : name(name)
	{
		ZE_GFX_ENABLE_ALL(gfx);
		ZE_GFX_THROW_FAILED(D3DReadFileToBlob(Utils::ToUtf8("Shaders\\" + name + ".cso").c_str(), &bytecode));
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateVertexShader(bytecode->GetBufferPointer(),
			bytecode->GetBufferSize(), nullptr, &vertexShader));
		ZE_GFX_SET_RID(vertexShader.Get());
	}
}
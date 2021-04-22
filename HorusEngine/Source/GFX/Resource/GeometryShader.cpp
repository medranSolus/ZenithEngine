#include "GFX/Resource/GeometryShader.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"
#include <d3dcompiler.h>

namespace GFX::Resource
{
	GeometryShader::GeometryShader(Graphics& gfx, const std::string& name) : name(name)
	{
		GFX_ENABLE_ALL(gfx);
		GFX_THROW_FAILED(D3DReadFileToBlob(Utils::ToUtf8("Shaders\\" + name + ".cso").c_str(), &bytecode));
		GFX_THROW_FAILED(GetDevice(gfx)->CreateGeometryShader(bytecode->GetBufferPointer(),
			bytecode->GetBufferSize(), nullptr, &geometryShader));
		GFX_SET_RID(geometryShader.Get());
	}
}
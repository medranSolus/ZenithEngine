#include "GeometryShader.h"
#include "GfxExceptionMacros.h"
#include "Utils.h"
#include <d3dcompiler.h>

namespace GFX::Resource
{
	GeometryShader::GeometryShader(Graphics& gfx, const std::string& path) : path(path)
	{
		GFX_ENABLE_ALL(gfx);

		GFX_THROW_FAILED(D3DReadFileToBlob(Utils::ToUtf8("Shaders\\" + path + ".cso").c_str(), &bytecode));
		GFX_THROW_FAILED(GetDevice(gfx)->CreateGeometryShader(bytecode->GetBufferPointer(),
			bytecode->GetBufferSize(), nullptr, &geometryShader));
	}
}
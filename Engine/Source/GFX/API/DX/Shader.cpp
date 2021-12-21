#include "GFX/API/DX/Shader.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX
{
	Shader::Shader(const std::wstring& name)
	{
		ZE_WIN_ENABLE_EXCEPT();
		ZE_WIN_THROW_FAILED(D3DReadFileToBlob((L"Shaders/" + name + L".cso").c_str(), &bytecode));
	}
}
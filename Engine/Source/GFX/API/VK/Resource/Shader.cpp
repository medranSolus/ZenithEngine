#include "GFX/API/VK/Resource/Shader.h"

namespace ZE::GFX::API::VK::Resource
{
	Shader::Shader(const std::wstring& name)
	{
#ifdef _ZE_MODE_DEBUG
		shaderName = Utils::ToAscii(name);
#endif
	}
}
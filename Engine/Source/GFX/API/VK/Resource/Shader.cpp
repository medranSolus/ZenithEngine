#include "GFX/API/VK/Resource/Shader.h"

namespace ZE::GFX::API::VK::Resource
{
	Shader::Shader(const std::wstring& name)
	{
#if _ZE_DEBUG_GFX_NAMES
		shaderName = Utils::ToAscii(name);
#endif
	}
}
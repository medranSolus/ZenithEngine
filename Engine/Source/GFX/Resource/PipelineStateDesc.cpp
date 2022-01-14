#include "GFX/Resource/PipelineStateDesc.h"

namespace ZE::GFX::Resource
{
	void PipelineStateDesc::SetShader(Shader*& shader, const wchar_t* name,
		std::unordered_map<std::wstring, Resource::Shader>& shaders) noexcept
	{
		if (shaders.find(name) == shaders.end())
			shader = &shaders.emplace(name, name).first->second;
		else
			shader = &shaders.at(name);
	}
}
#include "GFX/Resource/PipelineStateDesc.h"

namespace ZE::GFX::Resource
{
	void PipelineStateDesc::SetShader(Device& dev, Shader*& shader, std::string_view name,
		std::unordered_map<std::string, Resource::Shader>& shaders) noexcept
	{
		if (shaders.find(name.data()) == shaders.end())
			shader = &shaders.emplace(name, Shader{ dev, name }).first->second;
		else
			shader = &shaders.at(name.data());
	}
}
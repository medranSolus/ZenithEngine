#include "GFX/Resource/PipelineStateDesc.h"

namespace ZE::GFX::Resource
{
	Status PipelineStateDesc::SetShader(Device& dev, Shader*& shader, std::string_view name,
		std::unordered_map<std::string, Resource::Shader>& shaders) noexcept
	{
		if (shaders.find(name.data()) == shaders.end())
		{
			Shader newShader;
			ZE_EXPECT_RET_FAILED_CODE(newShader, Shader::Create(dev, name));
			shader = &shaders.emplace(name, std::move(newShader)).first->second;
		}
		else
			shader = &shaders.at(name.data());
		return {};
	}
}
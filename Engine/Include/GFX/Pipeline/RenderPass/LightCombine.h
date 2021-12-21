#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/DataBinding.h"

namespace ZE::GFX::Pipeline::RenderPass::LightCombine
{
	struct Resources
	{
		RID SSAO;
		RID LightColor;
		RID LightSpecular;
		RID GBufferColor;
		RID RenderTarget;
	};

	struct Data
	{
		//Resource::PipelineStateGfx PSO;
	};

	inline void Clean(void* data) { delete reinterpret_cast<Data*>(data); }

	Data* Setup(Device& dev, std::unordered_map<std::wstring, Resource::Shader>& shaders,
		std::map<U32, Resource::DataBindingDesc>& bindings, PixelFormat outputFormat);
	void Execute(CommandList& cl, PassData& passData);
}
#include "GFX/Pipeline/RenderPass/LightCombine.h"

namespace ZE::GFX::Pipeline::RenderPass::LightCombine
{
	Data* Setup(Device& dev, std::unordered_map<std::wstring, Resource::Shader>& shaders,
		std::map<U32, Resource::DataBindingDesc>& bindings, PixelFormat outputFormat)
	{
		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"FullscreenVS", shaders);
		psoDesc.SetShader(psoDesc.PS, L"LightCombinePS", shaders);
		psoDesc.Culling = Resource::CullMode::None;
		psoDesc.Topology = Resource::TopologyType::Triangle;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = outputFormat;

		auto bind = bindings.find(Resource::BindingDefaultID::LightCombine);
		if (bind == bindings.end())
		{
			Resource::DataBindingDesc desc;
			desc.Location = static_cast<U32>(bindings.size());
			desc.AddRange({ 1, 13, Resource::ShaderType::Pixel, Resource::BindingRangeFlags::CBV });
			desc.AddRange({ 1, 25, Resource::ShaderType::Pixel, Resource::BindingRangeFlags::SRV });
			desc.AddRange({ 1, 27, Resource::ShaderType::Pixel, Resource::BindingRangeFlags::SRV });
			desc.AddRange({ 1, 11, Resource::ShaderType::Pixel, Resource::BindingRangeFlags::CBV });
			bind = bindings.emplace(Resource::BindingDefaultID::LightCombine, desc).first;
		}
		return new Data{ /*{ dev, psoDesc, bind->second }*/ };
	}

	void Execute(CommandList& cl, PassData& passData)
	{
		Resources ids = *reinterpret_cast<const Resources*>(passData.Buffers);
		Data& data = *reinterpret_cast<Data*>(passData.OptData);
	}
}
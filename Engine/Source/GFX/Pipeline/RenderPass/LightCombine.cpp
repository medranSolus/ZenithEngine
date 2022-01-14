#include "GFX/Pipeline/RenderPass/LightCombine.h"

namespace ZE::GFX::Pipeline::RenderPass::LightCombine
{
	Data* Setup(Device& dev, RendererBuildData& buildData, PixelFormat outputFormat)
	{
		Data* passData = new Data;

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 13, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });
		desc.AddRange({ 1, 25, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.AddRange({ 5, 27, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPackAppend });
		desc.AddRange({ 1, 11, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });
		desc.Append(buildData.RendererSlots);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"FullscreenVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"LightCombinePS", buildData.ShaderCache);
		psoDesc.Stencil = Resource::StencilMode::DepthOff;
		psoDesc.Culling = Resource::CullMode::None;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = outputFormat;
		ZE_PSO_SET_NAME(psoDesc, "LightCombine");
		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		return passData;
	}

	void Execute(RendererExecuteData& renderData, PassData& passData)
	{
		Resources ids = *reinterpret_cast<const Resources*>(passData.Buffers);
		Data& data = *reinterpret_cast<Data*>(passData.OptData);

		renderData.CL.Open(renderData.Dev, data.State);
		renderData.Bindings.GetSchema(data.BindingIndex).SetGraphics(renderData.CL);
		renderData.Buffers.InitRTV(renderData.CL, ids.RenderTarget);

		renderData.Buffers.SetRTV(renderData.Dev, renderData.CL, ids.RenderTarget);
		renderData.CL.DrawFullscreen(renderData.Dev);

		renderData.CL.Close(renderData.Dev);
		renderData.Dev.ExecuteMain(renderData.CL);
	}
}
#include "GFX/Pipeline/RenderPass/SSAO.h"
#include "GFX/Resource/Constant.h"
#include "XeGTAO.h"

namespace ZE::GFX::Pipeline::RenderPass::SSAO
{
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData)
	{
		ExecuteData* passData = new ExecuteData;

		// Prefilter pass
		Binding::SchemaDesc desc;
		desc.AddRange({ 5, 0, Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack }); // Viewspace depth map
		desc.AddRange({ 1, 0, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Source depth map
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Compute);
		passData->BindingIndexPrefilter = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::Shader prefilter(L"SSAOPrefilterDepthCS");
		passData->StatePrefilter.Init(dev, prefilter, buildData.BindingLib.GetSchema(passData->BindingIndexPrefilter));

		// Main pass
		desc.Clear();
		desc.AddRange({ 1, 0, Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack }); // SSAO map
		desc.AddRange({ 1, 1, Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack }); // Depth edges
		desc.AddRange({ 1, 0, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Viewspace depth map
		desc.AddRange({ 1, 1, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Normal map
		desc.AddRange({ 1, 2, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack | Binding::RangeFlag::StaticData }); // Hilber LUT
		desc.AddRange({ 1, 12, Resource::ShaderType::Compute, Binding::RangeFlag::CBV | Binding::RangeFlag::GlobalBuffer }); // Renderer dynamic data
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Compute);
		passData->BindingIndexSSAO = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::Shader ssao(L"SSAOMainCS");
		passData->StateSSAO.Init(dev, ssao, buildData.BindingLib.GetSchema(passData->BindingIndexSSAO));

		// Denoise passes
		desc.Clear();
		desc.AddRange({ sizeof(U32), 0, Resource::ShaderType::Compute, Binding::RangeFlag::Constant }); // Is last denoise
		desc.AddRange({ 1, 0, Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack }); // SSAO map out
		desc.AddRange({ 1, 0, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Prev SSAO map
		desc.AddRange({ 1, 1, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Depth edges
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Compute);
		passData->BindingIndexDenoise = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::Shader denoise(L"SSAODenoiseCS");
		passData->StateDenoise.Init(dev, denoise, buildData.BindingLib.GetSchema(passData->BindingIndexDenoise));

		// Create Hilbert look-up texture
		Resource::Texture::PackDesc hilbertDesc;
		std::vector<Surface> surfaces;
		surfaces.emplace_back(XE_HILBERT_WIDTH, XE_HILBERT_WIDTH, PixelFormat::R16_UInt);

		U16* buffer = reinterpret_cast<U16*>(surfaces.front().GetBuffer());
		for (U32 x = 0; x < XE_HILBERT_WIDTH; ++x)
			for (U32 y = 0; y < XE_HILBERT_WIDTH; ++y)
				buffer[x + y * XE_HILBERT_WIDTH] = XeGTAO::HilbertIndex(x, y);

		hilbertDesc.Options = Resource::Texture::PackOption::StaticCreation;
		hilbertDesc.AddTexture(Resource::Texture::Type::Tex2D, Resource::Texture::Usage::NonPixelShader, std::move(surfaces));
		passData->HilbertLUT.Init(dev, hilbertDesc);
		dev.StartUpload();

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);
		const UInt2 size = renderData.Buffers.GetDimmensions(ids.Depth);

		ZE_DRAW_TAG_BEGIN(cl, L"SSAO", Pixel(0x89, 0xCF, 0xF0));

		Binding::Context prefilterCtx{ renderData.Bindings.GetSchema(data.BindingIndexPrefilter) };
		data.StatePrefilter.Bind(cl);
		prefilterCtx.BindingSchema.SetCompute(cl);

		renderData.Buffers.SetUAV(cl, prefilterCtx, ids.ViewspaceDepth, 0); // Bind 5 mip levels
		renderData.Buffers.SetSRV(cl, prefilterCtx, ids.Depth);
		renderData.SettingsBuffer.Bind(cl, prefilterCtx);
		cl.Compute(dev, Math::DivideRoundUp(size.x, 16U), Math::DivideRoundUp(size.y, 16U), 1);

		renderData.Buffers.BarrierTransition(cl, ids.ViewspaceDepth, Resource::StateUnorderedAccess, Resource::StateShaderResourceNonPS);

		Binding::Context mainCtx{ renderData.Bindings.GetSchema(data.BindingIndexSSAO) };
		data.StateSSAO.Bind(cl);
		mainCtx.BindingSchema.SetCompute(cl);
		renderData.Buffers.SetUAV(cl, mainCtx, ids.ScratchSSAO);
		renderData.Buffers.SetUAV(cl, mainCtx, ids.DepthEdges);
		renderData.Buffers.SetSRV(cl, mainCtx, ids.ViewspaceDepth);
		renderData.Buffers.SetSRV(cl, mainCtx, ids.Normal);
		data.HilbertLUT.Bind(cl, mainCtx);
		renderData.BindRendererDynamicData(cl, mainCtx);
		renderData.SettingsBuffer.Bind(cl, mainCtx);
		cl.Compute(dev, Math::DivideRoundUp(size.x, static_cast<U32>(XE_GTAO_NUMTHREADS_X)),
			Math::DivideRoundUp(size.y, static_cast<U32>(XE_GTAO_NUMTHREADS_Y)), 1);

		renderData.Buffers.BarrierTransition(cl,
			std::array
			{
				TransitionInfo(ids.ScratchSSAO, Resource::StateUnorderedAccess, Resource::StateShaderResourceNonPS),
				TransitionInfo(ids.DepthEdges, Resource::StateUnorderedAccess, Resource::StateShaderResourceNonPS),
				TransitionInfo(ids.ViewspaceDepth, Resource::StateShaderResourceNonPS, Resource::StateUnorderedAccess)
			});

		Binding::Context denoiseCtx{ renderData.Bindings.GetSchema(data.BindingIndexDenoise) };
		data.StateDenoise.Bind(cl);
		denoiseCtx.BindingSchema.SetCompute(cl);
		Resource::Constant<U32> lastDenoise(dev, true);
		lastDenoise.Bind(cl, denoiseCtx);
		renderData.Buffers.SetUAV(cl, denoiseCtx, ids.SSAO);
		renderData.Buffers.SetSRV(cl, denoiseCtx, ids.ScratchSSAO);
		renderData.Buffers.SetSRV(cl, denoiseCtx, ids.DepthEdges);
		renderData.SettingsBuffer.Bind(cl, denoiseCtx);
		cl.Compute(dev, Math::DivideRoundUp(size.x, XE_GTAO_NUMTHREADS_X * 2U),
			Math::DivideRoundUp(size.y, static_cast<U32>(XE_GTAO_NUMTHREADS_Y)), 1);

		renderData.Buffers.BarrierTransition(cl,
			std::array
			{
				TransitionInfo(ids.ScratchSSAO, Resource::StateShaderResourceNonPS, Resource::StateUnorderedAccess),
				TransitionInfo(ids.DepthEdges, Resource::StateShaderResourceNonPS, Resource::StateUnorderedAccess)
			});
		ZE_DRAW_TAG_END(cl);
	}
}
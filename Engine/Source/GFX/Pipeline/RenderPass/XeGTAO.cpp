#include "GFX/Pipeline/RenderPass/XeGTAO.h"
#include "GFX/Pipeline/RendererPBR.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::XeGTAO
{
	struct ConstantsXeGTAO
	{
		::XeGTAO::GTAOConstants Constants;
		float SliceCount;
		float StepsPerSlice;
	};

	void Clean(Device& dev, void* data) noexcept
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->ListChain.Exec([&dev](CommandList& cl) { cl.Free(dev); });
		execData->StatePrefilter.Free(dev);
		execData->StateAO.Free(dev);
		execData->StateDenoise.Free(dev);
		execData->HilbertLUT.Free(dev);
		delete execData;
	}

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData)
	{
		ExecuteData* passData = new ExecuteData;
		passData->ListChain.Exec([&dev](auto& x) { x.Init(dev, QueueType::Compute); });

		// Prefilter pass
		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 1, 3, Resource::ShaderType::Compute, Binding::RangeFlag::CBV }); // XeGTAO constants
		desc.AddRange({ 5, 0, 1, Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack }); // Viewspace depth map
		desc.AddRange({ 1, 0, 2, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Source depth map
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Compute);
		passData->BindingIndexPrefilter = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::Shader prefilter(dev, "XeGTAOPrefilterDepthCS");
		passData->StatePrefilter.Init(dev, prefilter, buildData.BindingLib.GetSchema(passData->BindingIndexPrefilter));
		prefilter.Free(dev);

		// Main pass
		desc.Clear();
		desc.AddRange({ 1, 1, 7, Resource::ShaderType::Compute, Binding::RangeFlag::CBV }); // XeGTAO constants
		desc.AddRange({ 1, 0, 2, Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack }); // AO map
		desc.AddRange({ 1, 1, 3, Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack }); // Depth edges
		desc.AddRange({ 1, 0, 4, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Viewspace depth map
		desc.AddRange({ 1, 1, 5, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Normal map
		desc.AddRange({ 1, 2, 6, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack | Binding::RangeFlag::StaticData }); // Hilber LUT
		desc.AddRange({ 1, 12, 1, Resource::ShaderType::Compute, Binding::RangeFlag::CBV | Binding::RangeFlag::GlobalBuffer }); // Renderer dynamic data
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Compute);
		passData->BindingIndexAO = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::Shader ao(dev, "XeGTAOMainCS");
		passData->StateAO.Init(dev, ao, buildData.BindingLib.GetSchema(passData->BindingIndexAO));
		ao.Free(dev);

		// Denoise passes
		desc.Clear();
		desc.AddRange({ sizeof(U32), 0, 0, Resource::ShaderType::Compute, Binding::RangeFlag::Constant }); // Is last denoise
		desc.AddRange({ 1, 1, 4, Resource::ShaderType::Compute, Binding::RangeFlag::CBV }); // XeGTAO constants
		desc.AddRange({ 1, 0, 3, Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack }); // AO map out
		desc.AddRange({ 1, 0, 4, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Prev AO map
		desc.AddRange({ 1, 1, 2, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Depth edges
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Compute);
		passData->BindingIndexDenoise = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::Shader denoise(dev, "XeGTAODenoiseCS");
		passData->StateDenoise.Init(dev, denoise, buildData.BindingLib.GetSchema(passData->BindingIndexDenoise));
		denoise.Free(dev);

		// Create Hilbert look-up texture
		Resource::Texture::PackDesc hilbertDesc;
		std::vector<Surface> surfaces;
		surfaces.emplace_back(XE_HILBERT_WIDTH, XE_HILBERT_WIDTH, PixelFormat::R16_UInt);

		U16* buffer = reinterpret_cast<U16*>(surfaces.front().GetBuffer());
		for (U32 y = 0; y < XE_HILBERT_WIDTH; ++y)
			for (U32 x = 0; x < XE_HILBERT_WIDTH; ++x)
				buffer[x + y * XE_HILBERT_WIDTH] = Utils::SafeCast<U16>(::XeGTAO::HilbertIndex(x, y));

		hilbertDesc.Options = Resource::Texture::PackOption::StaticCreation;
		hilbertDesc.AddTexture(Resource::Texture::Type::Tex2D, Resource::Texture::Usage::NonPixelShader, std::move(surfaces));
		passData->HilbertLUT.Init(dev, hilbertDesc);

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("XeGTAO");
		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);
		const UInt2 size = renderData.Buffers.GetDimmensions(ids.Depth);

		CommandList& list = data.ListChain.Get();
		list.Reset(dev);
		list.Open(dev, data.StatePrefilter);
		ZE_DRAW_TAG_BEGIN(dev, list, "XeGTAO", Pixel(0x89, 0xCF, 0xF0));

		RendererPBR& renderer = *reinterpret_cast<RendererPBR*>(renderData.Renderer);
		auto& cbuffer = *renderData.DynamicBuffer;

		XeGTAOSettings& settings = renderer.GetXeGTAOSettings();
		ConstantsXeGTAO constants = {};
		constants.SliceCount = settings.SliceCount;
		constants.StepsPerSlice = settings.StepsPerSlice;
		::XeGTAO::GTAOUpdateConstants(constants.Constants, size.X, size.Y, settings.Settings,
			reinterpret_cast<const float*>(&renderer.GetProjection()), true, static_cast<U32>(Settings::GetFrameIndex()));
		auto cbufferInfo = cbuffer.Alloc(dev, &constants, sizeof(ConstantsXeGTAO));

		Binding::Context prefilterCtx{ renderData.Bindings.GetSchema(data.BindingIndexPrefilter) };
		prefilterCtx.BindingSchema.SetCompute(list);
		cbuffer.Bind(list, prefilterCtx, cbufferInfo);
		renderData.Buffers.SetUAV(list, prefilterCtx, ids.ViewspaceDepth, 0); // Bind 5 mip levels
		renderData.Buffers.SetSRV(list, prefilterCtx, ids.Depth);
		renderData.SettingsBuffer.Bind(list, prefilterCtx);
		list.Compute(dev, Math::DivideRoundUp(size.X, 16U), Math::DivideRoundUp(size.Y, 16U), 1);

		renderData.Buffers.BarrierTransition(list, ids.ViewspaceDepth, Resource::StateUnorderedAccess, Resource::StateShaderResourceNonPS);

		Binding::Context mainCtx{ renderData.Bindings.GetSchema(data.BindingIndexAO) };
		data.StateAO.Bind(list);
		mainCtx.BindingSchema.SetCompute(list);
		cbuffer.Bind(list, mainCtx, cbufferInfo);
		renderData.Buffers.SetUAV(list, mainCtx, ids.ScratchAO);
		renderData.Buffers.SetUAV(list, mainCtx, ids.DepthEdges);
		renderData.Buffers.SetSRV(list, mainCtx, ids.ViewspaceDepth);
		renderData.Buffers.SetSRV(list, mainCtx, ids.Normal);
		data.HilbertLUT.Bind(list, mainCtx);
		renderData.BindRendererDynamicData(list, mainCtx);
		renderData.SettingsBuffer.Bind(list, mainCtx);
		list.Compute(dev, Math::DivideRoundUp(size.X, static_cast<U32>(XE_GTAO_NUMTHREADS_X)),
			Math::DivideRoundUp(size.Y, static_cast<U32>(XE_GTAO_NUMTHREADS_Y)), 1);

		renderData.Buffers.BarrierTransition(list, std::array
			{
				TransitionInfo(ids.ScratchAO, Resource::StateUnorderedAccess, Resource::StateShaderResourceNonPS),
					TransitionInfo(ids.DepthEdges, Resource::StateUnorderedAccess, Resource::StateShaderResourceNonPS),
					TransitionInfo(ids.ViewspaceDepth, Resource::StateShaderResourceNonPS, Resource::StateUnorderedAccess)
			});

		Binding::Context denoiseCtx{ renderData.Bindings.GetSchema(data.BindingIndexDenoise) };
		data.StateDenoise.Bind(list);
		denoiseCtx.BindingSchema.SetCompute(list);
		Resource::Constant<U32> lastDenoise(dev, true);
		lastDenoise.Bind(list, denoiseCtx);
		cbuffer.Bind(list, denoiseCtx, cbufferInfo);
		renderData.Buffers.SetUAV(list, denoiseCtx, ids.AO);
		renderData.Buffers.SetSRV(list, denoiseCtx, ids.ScratchAO);
		renderData.Buffers.SetSRV(list, denoiseCtx, ids.DepthEdges);
		renderData.SettingsBuffer.Bind(list, denoiseCtx);
		list.Compute(dev, Math::DivideRoundUp(size.X, XE_GTAO_NUMTHREADS_X * 2U),
			Math::DivideRoundUp(size.Y, static_cast<U32>(XE_GTAO_NUMTHREADS_Y)), 1);

		renderData.Buffers.BarrierTransition(list, std::array
			{
				TransitionInfo(ids.ScratchAO, Resource::StateShaderResourceNonPS, Resource::StateUnorderedAccess),
					TransitionInfo(ids.DepthEdges, Resource::StateShaderResourceNonPS, Resource::StateUnorderedAccess)
			});

		ZE_DRAW_TAG_END(dev, list);
		list.Close(dev);
		dev.ExecuteCompute(list);
	}
}
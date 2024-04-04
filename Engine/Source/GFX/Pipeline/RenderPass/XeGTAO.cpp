#include "GFX/Pipeline/RenderPass/XeGTAO.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::XeGTAO
{
#pragma pack(push, 1)
	struct ConstantsXeGTAO
	{
		::XeGTAO::GTAOConstants Constants;
		float SliceCount;
		float StepsPerSlice;
	};
#pragma pack(pop)

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void*& initData) { return Initialize(dev, buildData); }

	void UpdateQualityInfo(ExecuteData& passData) noexcept
	{
		switch (passData.Settings.QualityLevel)
		{
		default:
			ZE_FAIL("Unknown XeGTAO quality level!");
			[[fallthrough]];
		case 0: // Low
		{
			passData.SliceCount = 1.0f;
			passData.StepsPerSlice = 2.0f;
			break;
		}
		case 1: // Medium
		{
			passData.SliceCount = 2.0f;
			passData.StepsPerSlice = 2.0f;
			break;
		}
		case 2: // High
		{
			passData.SliceCount = 3.0f;
			passData.StepsPerSlice = 3.0f;
			break;
		}
		case 3: // Ultra
		{
			passData.SliceCount = 9.0f;
			passData.StepsPerSlice = 3.0f;
			break;
		}
		}
	}

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ static_cast<PassType>(CorePassType::XeGTAO) };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Clean = Clean;
		return desc;
	}

	void Clean(Device& dev, void* data) noexcept
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->StatePrefilter.Free(dev);
		execData->StateAO.Free(dev);
		execData->StateDenoise.Free(dev);
		execData->HilbertLUT.Free(dev);
		delete execData;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData)
	{
		ExecuteData* passData = new ExecuteData;
		passData->Settings.QualityLevel = 3;
		passData->Settings.DenoisePasses = 1;
		UpdateQualityInfo(*passData);

		// Prefilter pass
		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 1, 3, Resource::ShaderType::Compute, Binding::RangeFlag::CBV }); // XeGTAO constants
		desc.AddRange({ 5, 0, 1, Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack }); // Viewspace depth map
		desc.AddRange({ 1, 0, 2, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Source depth map
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Compute);
		desc.AppendSamplers(buildData.Samplers);
		passData->BindingIndexPrefilter = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::Shader prefilter(dev, "XeGTAOPrefilterDepthCS");
		passData->StatePrefilter.Init(dev, prefilter, buildData.BindingLib.GetSchema(passData->BindingIndexPrefilter));
		prefilter.Free(dev);

		// Main pass
		desc.Ranges.clear();
		desc.AddRange({ 1, 1, 7, Resource::ShaderType::Compute, Binding::RangeFlag::CBV }); // XeGTAO constants
		desc.AddRange({ 1, 0, 2, Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack }); // AO map
		desc.AddRange({ 1, 1, 3, Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack }); // Depth edges
		desc.AddRange({ 1, 0, 4, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Viewspace depth map
		desc.AddRange({ 1, 1, 5, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Normal map
		desc.AddRange({ 1, 2, 6, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack | Binding::RangeFlag::StaticData }); // Hilber LUT
		desc.AddRange(buildData.DynamicDataRange, Resource::ShaderType::Compute);
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Compute);
		passData->BindingIndexAO = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::Shader ao(dev, "XeGTAOMainCS");
		passData->StateAO.Init(dev, ao, buildData.BindingLib.GetSchema(passData->BindingIndexAO));
		ao.Free(dev);

		// Denoise passes
		desc.Ranges.clear();
		desc.AddRange({ sizeof(U32), 0, 0, Resource::ShaderType::Compute, Binding::RangeFlag::Constant }); // Is last denoise
		desc.AddRange({ 1, 1, 4, Resource::ShaderType::Compute, Binding::RangeFlag::CBV }); // XeGTAO constants
		desc.AddRange({ 1, 0, 3, Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack }); // AO map out
		desc.AddRange({ 1, 0, 4, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Prev AO map
		desc.AddRange({ 1, 1, 2, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Depth edges
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Compute);
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
				buffer[x + y * Math::AlignUp(static_cast<U64>(XE_HILBERT_WIDTH), Surface::ROW_PITCH_ALIGNMENT / sizeof(U16))] = Utils::SafeCast<U16>(::XeGTAO::HilbertIndex(x, y));

		hilbertDesc.Options = Resource::Texture::PackOption::StaticCreation;
		hilbertDesc.AddTexture(Resource::Texture::Type::Tex2D, std::move(surfaces));
		passData->HilbertLUT.Init(dev, buildData.Assets.GetDisk(), hilbertDesc);

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("XeGTAO");
		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();
		const UInt2 size = {};// renderData.Buffers.GetDimmensions(ids.Depth);

		ZE_DRAW_TAG_BEGIN(dev, cl, "XeGTAO", Pixel(0x89, 0xCF, 0xF0));
		auto& cbuffer = *renderData.DynamicBuffer;

		ConstantsXeGTAO constants = {};
		constants.SliceCount = data.SliceCount;
		constants.StepsPerSlice = data.StepsPerSlice;
		::XeGTAO::GTAOUpdateConstants(constants.Constants, size.X, size.Y, data.Settings,
			reinterpret_cast<const float*>(&renderData.GraphData.Projection), true, static_cast<U32>(Settings::GetFrameIndex()));
		auto cbufferInfo = cbuffer.Alloc(dev, &constants, sizeof(ConstantsXeGTAO));

		Binding::Context prefilterCtx{ renderData.Bindings.GetSchema(data.BindingIndexPrefilter) };
		prefilterCtx.BindingSchema.SetCompute(cl);
		cbuffer.Bind(cl, prefilterCtx, cbufferInfo);
		//renderData.Buffers.SetUAV(cl, prefilterCtx, ids.ViewspaceDepth, 0); // Bind 5 mip levels
		//renderData.Buffers.SetSRV(cl, prefilterCtx, ids.Depth);
		renderData.SettingsBuffer.Bind(cl, prefilterCtx);
		cl.Compute(dev, Math::DivideRoundUp(size.X, 16U), Math::DivideRoundUp(size.Y, 16U), 1);

		//renderData.Buffers.BarrierTransition(cl, ids.ViewspaceDepth, Resource::StateUnorderedAccess, Resource::StateShaderResourceNonPS);

		Binding::Context mainCtx{ renderData.Bindings.GetSchema(data.BindingIndexAO) };
		data.StateAO.Bind(cl);
		mainCtx.BindingSchema.SetCompute(cl);
		cbuffer.Bind(cl, mainCtx, cbufferInfo);
		//renderData.Buffers.SetUAV(cl, mainCtx, ids.ScratchAO);
		//renderData.Buffers.SetUAV(cl, mainCtx, ids.DepthEdges);
		//renderData.Buffers.SetSRV(cl, mainCtx, ids.ViewspaceDepth);
		//renderData.Buffers.SetSRV(cl, mainCtx, ids.Normal);
		data.HilbertLUT.Bind(cl, mainCtx);
		renderData.BindRendererDynamicData(cl, mainCtx);
		renderData.SettingsBuffer.Bind(cl, mainCtx);
		cl.Compute(dev, Math::DivideRoundUp(size.X, static_cast<U32>(XE_GTAO_NUMTHREADS_X)),
			Math::DivideRoundUp(size.Y, static_cast<U32>(XE_GTAO_NUMTHREADS_Y)), 1);

		/*
		renderData.Buffers.BarrierTransition(cl, std::array
			{
				TransitionInfo(ids.ScratchAO, Resource::StateUnorderedAccess, Resource::StateShaderResourceNonPS),
					TransitionInfo(ids.DepthEdges, Resource::StateUnorderedAccess, Resource::StateShaderResourceNonPS),
					TransitionInfo(ids.ViewspaceDepth, Resource::StateShaderResourceNonPS, Resource::StateUnorderedAccess)
			});
		*/

		Binding::Context denoiseCtx{ renderData.Bindings.GetSchema(data.BindingIndexDenoise) };
		data.StateDenoise.Bind(cl);
		denoiseCtx.BindingSchema.SetCompute(cl);
		Resource::Constant<U32> lastDenoise(dev, true);
		lastDenoise.Bind(cl, denoiseCtx);
		cbuffer.Bind(cl, denoiseCtx, cbufferInfo);
		//renderData.Buffers.SetUAV(cl, denoiseCtx, ids.AO);
		//renderData.Buffers.SetSRV(cl, denoiseCtx, ids.ScratchAO);
		//renderData.Buffers.SetSRV(cl, denoiseCtx, ids.DepthEdges);
		renderData.SettingsBuffer.Bind(cl, denoiseCtx);
		cl.Compute(dev, Math::DivideRoundUp(size.X, XE_GTAO_NUMTHREADS_X * 2U),
			Math::DivideRoundUp(size.Y, static_cast<U32>(XE_GTAO_NUMTHREADS_Y)), 1);

		/*
		renderData.Buffers.BarrierTransition(cl, std::array
			{
				TransitionInfo(ids.ScratchAO, Resource::StateShaderResourceNonPS, Resource::StateUnorderedAccess),
					TransitionInfo(ids.DepthEdges, Resource::StateShaderResourceNonPS, Resource::StateUnorderedAccess)
			});
		*/
		ZE_DRAW_TAG_END(dev, cl);
	}
}
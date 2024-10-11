#include "GFX/Pipeline/RenderPass/SSSR.h"
#include "GFX/FfxBackendInterface.h"

namespace ZE::GFX::Pipeline::RenderPass::SSSR
{
	static bool Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { Update(dev, buildData.FfxInterface, *reinterpret_cast<ExecuteData*>(passData)); return false; }

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) { return Initialize(dev, buildData.FfxInterface); }

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ Base(CorePassType::SSSR) };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.Clean = Clean;
		return desc;
	}

	void Clean(Device& dev, void* data) noexcept
	{
		ZE_FFX_ENABLE();
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		ZE_FFX_CHECK(ffxSssrContextDestroy(&execData->Ctx), "Error destroying SSSR context!");
		delete execData;
	}

	void Update(Device& dev, const FfxInterface& ffxInterface, ExecuteData& passData, bool firstUpdate)
	{
		if (passData.RenderSize != Settings::RenderSize)
		{
			ZE_FFX_ENABLE();
			passData.RenderSize = Settings::RenderSize;

			if (firstUpdate)
			{
				ZE_FFX_CHECK(ffxSssrContextDestroy(&passData.Ctx), "Error destroying SSSR context!");
			}
			FfxSssrContextDescription sssrDesc = {};
			sssrDesc.flags = FFX_SSSR_ENABLE_DEPTH_INVERTED;
			sssrDesc.renderSize.width = passData.RenderSize.X;
			sssrDesc.renderSize.height = passData.RenderSize.Y;
			sssrDesc.normalsHistoryBufferFormat = FFX::GetSurfaceFormat(PixelFormat::R16G16_Float);
			sssrDesc.backendInterface = ffxInterface;
			ZE_FFX_THROW_FAILED(ffxSssrContextCreate(&passData.Ctx, &sssrDesc), "Error creating SSSR context!");
		}
	}

	void* Initialize(Device& dev, const FfxInterface& ffxInterface)
	{
		ExecuteData* passData = new ExecuteData;
		Update(dev, ffxInterface, *passData, true);
		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_FFX_ENABLE();
		ZE_PERF_GUARD("SSSR");

		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();
		const UInt2 inputSize = renderData.Buffers.GetDimmensions(ids.Color);

		ZE_DRAW_TAG_BEGIN(dev, cl, "SSSR", Pixel(0x80, 0x00, 0x00));

		FfxSssrDispatchDescription desc = {};
		desc.commandList = FFX::GetCommandList(cl);

		desc.color = FFX::GetResource(renderData.Buffers, ids.Color, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.depth = FFX::GetResource(renderData.Buffers, ids.Depth, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.motionVectors = FFX::GetResource(renderData.Buffers, ids.MotionVectors, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.normal = FFX::GetResource(renderData.Buffers, ids.NormalMap, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.materialParameters = FFX::GetResource(renderData.Buffers, ids.MaterialData, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.environmentMap = FFX::GetResource(renderData.Buffers, ids.EnvironmentMap, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.brdfTexture = FFX::GetResource(renderData.Buffers, ids.BrdfLut, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.output = FFX::GetResource(renderData.Buffers, ids.SSSR, FFX_RESOURCE_STATE_COMPUTE_READ);

		*reinterpret_cast<Float4x4*>(desc.invViewProjection) = renderData.DynamicData.ViewProjectionInverseTps;
		Math::XMStoreFloat4x4(reinterpret_cast<Float4x4*>(desc.projection), Math::XMLoadFloat4x4(&renderData.GraphData.Projection));
		*reinterpret_cast<Float4x4*>(desc.invProjection) = renderData.DynamicData.ViewProjectionInverseTps;
		*reinterpret_cast<Float4x4*>(desc.view) = renderData.DynamicData.ViewTps;
		Math::XMStoreFloat4x4(reinterpret_cast<Float4x4*>(desc.invView), Math::XMMatrixInverse(nullptr, Math::XMLoadFloat4x4(&renderData.DynamicData.ViewTps)));
		*reinterpret_cast<Float4x4*>(desc.prevViewProjection) = renderData.GraphData.PrevViewProjectionTps;

		desc.renderSize = { inputSize.X, inputSize.Y };
		desc.motionVectorScale.x = -0.5f;
		desc.motionVectorScale.y = -0.5f;
		desc.iblFactor = data.IblFactor;
		// Custom way of loading normals is chosen so no need to perform any unpacking from SDK (custom callbacks provided)
		desc.normalUnPackMul = 1.0f;
		desc.normalUnPackAdd = 0.0f;
		desc.roughnessChannel = 0; // Not used, specified directly in shader
		desc.isRoughnessPerceptual = true; // Not used, all shaders assume roughness is linear
		desc.temporalStabilityFactor = data.TemporalStabilityFactor;
		desc.depthBufferThickness = data.DepthBufferThickness;
		desc.roughnessThreshold = data.RoughnessThreshold;
		desc.varianceThreshold = data.VarianceThreshold;
		desc.maxTraversalIntersections = data.MaxTraversalIntersections;
		desc.minTraversalOccupancy = data.MinTraversalOccupancy;
		desc.mostDetailedMip = data.MostDetailedMip;
		desc.samplesPerQuad = data.SamplesPerQuad;
		desc.temporalVarianceGuidedTracingEnabled = data.TemporalVarianceGuidedTracingEnabled;
		ZE_FFX_THROW_FAILED(ffxSssrContextDispatch(&data.Ctx, &desc), "Error performing SSSR!");

		ZE_DRAW_TAG_END(dev, cl);
	}
}
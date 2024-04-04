#include "GFX/Pipeline/RenderPass/SSSR.h"
#include "GFX/FfxBackendInterface.h"

namespace ZE::GFX::Pipeline::RenderPass::SSSR
{
	static bool Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { Update(dev, *reinterpret_cast<ExecuteData*>(passData)); return false; }

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void*& initData) { return Initialize(dev, buildData); }

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ static_cast<PassType>(CorePassType::SSSR) };
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

	void Update(Device& dev, ExecuteData& passData, bool firstUpdate)
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
			sssrDesc.normalsHistoryBufferFormat = GetFfxSurfaceFormat(PixelFormat::R16G16_Float);
			sssrDesc.backendInterface = dev.GetFfxInterface();
			ZE_FFX_THROW_FAILED(ffxSssrContextCreate(&passData.Ctx, &sssrDesc), "Error creating SSSR context!");
		}
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData)
	{
		ExecuteData* passData = new ExecuteData;
		Update(dev, *passData, true);
		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		return;
		ZE_FFX_ENABLE();
		ZE_PERF_GUARD("SSSR");

		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();
		const UInt2 inputSize = {};// renderData.Buffers.GetDimmensions(ids.Color);

		ZE_DRAW_TAG_BEGIN(dev, cl, "SSSR", Pixel(0x80, 0x00, 0x00));

		// https://gpuopen.com/manuals/fidelityfx_sdk/fidelityfx_sdk-page_techniques_stochastic-screen-space-reflections

		FfxSssrDispatchDescription desc = {};
		desc.commandList = ffxGetCommandList(cl);

		Resource::Generic color, depth, motion, normal, roughness, environment, brdf, sssr;
		//desc.color = ffxGetResource(renderData.Buffers, color, ids.Color, Resource::StateShaderResourceNonPS);
		//desc.depth = ffxGetResource(renderData.Buffers, depth, ids.Depth, Resource::StateShaderResourceNonPS);
		//desc.motionVectors = ffxGetResource(renderData.Buffers, motion, ids.MotionVectors, Resource::StateShaderResourceNonPS);
		//desc.normal = ffxGetResource(renderData.Buffers, normal, ids.NormalMap, Resource::StateShaderResourceNonPS);
		//desc.materialParameters = ffxGetResource(renderData.Buffers, roughness, ids.Roughness, Resource::StateShaderResourceNonPS);
		//desc.environmentMap = ffxGetResource(renderData.Buffers, environment, ids.EnvironmentMap, Resource::StateShaderResourceNonPS);
		//desc.brdfTexture = ffxGetResource(renderData.Buffers, brdf, ids.BrdfLut, Resource::StateShaderResourceNonPS);
		//desc.output = ffxGetResource(renderData.Buffers, sssr, ids.SSSR, Resource::StateShaderResourceNonPS);

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
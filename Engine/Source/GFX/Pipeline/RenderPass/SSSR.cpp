#include "GFX/Pipeline/RenderPass/SSSR.h"
#include "GFX/Pipeline/RendererPBR.h"
#include "GFX/FfxBackendInterface.h"

namespace ZE::GFX::Pipeline::RenderPass::SSSR
{
	void Clean(Device& dev, void* data) noexcept
	{
		//ZE_FFX_ENABLE();
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		//ZE_FFX_CHECK(ffxSssrContextDestroy(&execData->Ctx), "Error destroying SSSR context!");
		//execData->ListChain.Exec([&dev](CommandList& cl) { cl.Free(dev); });
		delete execData;
	}

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, U32 renderWidth, U32 renderHeight)
	{
		ZE_FFX_ENABLE();
		ExecuteData* passData = new ExecuteData;
		return passData;

		FfxSssrContextDescription sssrDesc = {};
		sssrDesc.flags = FFX_SSSR_ENABLE_DEPTH_INVERTED;
		sssrDesc.renderSize.width = renderWidth;
		sssrDesc.renderSize.height = renderHeight;
		sssrDesc.normalsHistoryBufferFormat = FFX_SURFACE_FORMAT_R16G16B16A16_FLOAT; // TODO ?
		sssrDesc.backendInterface = dev.GetFfxInterface();
		ZE_FFX_THROW_FAILED(ffxSssrContextCreate(&passData->Ctx, &sssrDesc), "Error creating SSSR context!");

		passData->ListChain.Exec([&dev](auto& x) { x.Init(dev, QueueType::Compute); });

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		return;
		ZE_FFX_ENABLE();
		ZE_PERF_GUARD("SSSR");

		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);
		const UInt2 inputSize = renderData.Buffers.GetDimmensions(ids.Color);

		CommandList& list = data.ListChain.Get();
		list.Reset(dev);
		list.Open(dev);
		ZE_DRAW_TAG_BEGIN(dev, list, "SSSR", Pixel(0x80, 0x00, 0x00));

		RendererPBR& renderer = *reinterpret_cast<RendererPBR*>(renderData.Renderer);
		const CameraPBR& dynamicData = *reinterpret_cast<CameraPBR*>(renderData.DynamicData);

		// https://gpuopen.com/manuals/fidelityfx_sdk/fidelityfx_sdk-page_techniques_stochastic-screen-space-reflections

		FfxSssrDispatchDescription desc = {};
		desc.commandList = ffxGetCommandList(list);

		Resource::Generic color, depth, motion, normal, roughness, environment, brdf, sssr;
		desc.color = ffxGetResource(renderData.Buffers, color, ids.Color, Resource::StateShaderResourceNonPS);
		desc.depth = ffxGetResource(renderData.Buffers, depth, ids.Depth, Resource::StateShaderResourceNonPS);
		desc.motionVectors = ffxGetResource(renderData.Buffers, motion, ids.MotionVectors, Resource::StateShaderResourceNonPS);
		desc.normal = ffxGetResource(renderData.Buffers, normal, ids.NormalMap, Resource::StateShaderResourceNonPS);
		desc.materialParameters = ffxGetResource(renderData.Buffers, roughness, ids.Roughness, Resource::StateShaderResourceNonPS);
		desc.environmentMap = ffxGetResource(renderData.Buffers, environment, ids.EnvironmentMap, Resource::StateShaderResourceNonPS);
		desc.brdfTexture = ffxGetResource(renderData.Buffers, brdf, ids.BrdfLut, Resource::StateShaderResourceNonPS);
		desc.output = ffxGetResource(renderData.Buffers, sssr, ids.SSSR, Resource::StateShaderResourceNonPS);

		*reinterpret_cast<Float4x4*>(desc.invViewProjection) = dynamicData.ViewProjectionInverseTps;
		Math::XMStoreFloat4x4(reinterpret_cast<Float4x4*>(desc.projection), Math::XMLoadFloat4x4(&renderer.GetProjection()));
		*reinterpret_cast<Float4x4*>(desc.invProjection) = dynamicData.ViewProjectionInverseTps;
		*reinterpret_cast<Float4x4*>(desc.view) = dynamicData.ViewTps;
		Math::XMStoreFloat4x4(reinterpret_cast<Float4x4*>(desc.invView), Math::XMMatrixInverse(nullptr, Math::XMLoadFloat4x4(&dynamicData.ViewTps)));
		*reinterpret_cast<Float4x4*>(desc.prevViewProjection) = renderer.GetPrevViewProjectionTps();

		desc.renderSize = { inputSize.X, inputSize.Y };
		desc.motionVectorScale.x = -0.5f;
		desc.motionVectorScale.y = -0.5f;
		desc.iblFactor = renderer.GetIBLFactor();
		// Custom way of loading normals is chosen so no need to perform any unpacking from SDK (custom callbacks provided)
		desc.normalUnPackMul = 1.0f;
		desc.normalUnPackAdd = 0.0f;
		desc.roughnessChannel = 1;
		desc.isRoughnessPerceptual = true;
		desc.temporalStabilityFactor = renderer.GetSSSRSettings().TemporalStabilityFactor;
		desc.depthBufferThickness = renderer.GetSSSRSettings().DepthBufferThickness;
		desc.roughnessThreshold = renderer.GetSSSRSettings().RoughnessThreshold;
		desc.varianceThreshold = renderer.GetSSSRSettings().VarianceThreshold;
		desc.maxTraversalIntersections = renderer.GetSSSRSettings().MaxTraversalIntersections;
		desc.minTraversalOccupancy = renderer.GetSSSRSettings().MinTraversalOccupancy;
		desc.mostDetailedMip = renderer.GetSSSRSettings().MostDetailedMip;
		desc.samplesPerQuad = renderer.GetSSSRSettings().SamplesPerQuad;
		desc.temporalVarianceGuidedTracingEnabled = renderer.GetSSSRSettings().TemporalVarianceGuidedTracingEnabled;
		ZE_FFX_THROW_FAILED(ffxSssrContextDispatch(&data.Ctx, &desc), "Error performing SSSR!");

		ZE_DRAW_TAG_END(dev, list);
		list.Close(dev);
		dev.ExecuteCompute(list);
	}
}
#include "GFX/Pipeline/RenderPass/CACAO.h"
#include "GFX/Pipeline/RendererPBR.h"
#include "GFX/FfxBackendInterface.h"

namespace ZE::GFX::Pipeline::RenderPass::CACAO
{
	void Clean(Device& dev, void* data) noexcept
	{
		ZE_FFX_ENABLE();
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		ZE_FFX_CHECK(ffxCacaoContextDestroy(&execData->Ctx), "Error destroying CACAO context!");
		execData->ListChain.Exec([&dev](CommandList& cl) { cl.Free(dev); });
		delete execData;
	}

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, U32 renderWidth, U32 renderHeight)
	{
		ZE_FFX_ENABLE();
		ExecuteData* passData = new ExecuteData;

		FfxCacaoContextDescription cacaoDesc = {};
		cacaoDesc.backendInterface = dev.GetFfxInterface();
		cacaoDesc.width = renderWidth;
		cacaoDesc.height = renderHeight;
		cacaoDesc.useDownsampledSsao = false;
		ZE_FFX_THROW_FAILED(ffxCacaoContextCreate(&passData->Ctx, &cacaoDesc), "Error creating CACAO context!");

		passData->ListChain.Exec([&dev](auto& x) { x.Init(dev, QueueType::Compute); });

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ZE_FFX_ENABLE();
		ZE_PERF_GUARD("CACAO");

		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		CommandList& list = data.ListChain.Get();
		list.Reset(dev);
		list.Open(dev);
		ZE_DRAW_TAG_BEGIN(dev, list, "CACAO", Pixel(0x89, 0xCF, 0xF0));

		RendererPBR& renderer = *reinterpret_cast<RendererPBR*>(renderData.Renderer);
		const CameraPBR& dynamicData = *reinterpret_cast<CameraPBR*>(renderData.DynamicData);

		FfxCacaoSettings& settings = renderer.GetCacaoSettings();
		settings.temporalSupersamplingAngleOffset = Math::PI * Utils::SafeCast<float>(Settings::GetFrameIndex() % 3) / 3.0f;
		settings.temporalSupersamplingRadiusOffset = 1.0f + (Utils::SafeCast<float>(Settings::GetFrameIndex() % 3) - 1.0f) / 30.0f;
		ZE_FFX_CHECK(ffxCacaoUpdateSettings(&data.Ctx, &settings, false), "Error updating CACAO settings!");

		FfxCacaoDispatchDescription desc = {};
		desc.commandList = ffxGetCommandList(list);
		Resource::Generic depth, normal, ao;
		desc.depthBuffer = ffxGetResource(renderData.Buffers, depth, ids.Depth, Resource::StateShaderResourceNonPS);
		desc.normalBuffer = ffxGetResource(renderData.Buffers, normal, ids.Normal, Resource::StateShaderResourceNonPS);
		desc.outputBuffer = ffxGetResource(renderData.Buffers, ao, ids.AO, Resource::StateUnorderedAccess);
		// Matrix data is not modified inside callbacks, missing const specifier in header
		desc.proj = const_cast<FfxCacaoMat4x4*>(reinterpret_cast<const FfxCacaoMat4x4*>(&renderer.GetProjection()));
		desc.normalsToView = const_cast<FfxCacaoMat4x4*>(reinterpret_cast<const FfxCacaoMat4x4*>(&dynamicData.ViewTps));
		// Custom way of loading normals is chosen so no need to perform any unpacking from SDK (custom callbacks provided)
		desc.normalUnpackMul = 1.0f;
		desc.normalUnpackAdd = 0.0f;
		ZE_FFX_THROW_FAILED(ffxCacaoContextDispatch(&data.Ctx, &desc), "Error performing CACAO!");

		ZE_DRAW_TAG_END(dev, list);
		list.Close(dev);
		dev.ExecuteCompute(list);
	}
}
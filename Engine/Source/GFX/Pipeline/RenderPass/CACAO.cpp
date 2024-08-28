#include "GFX/Pipeline/RenderPass/CACAO.h"
#include "GFX/FfxBackendInterface.h"

namespace ZE::GFX::Pipeline::RenderPass::CACAO
{
	static bool Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { Update(dev, *reinterpret_cast<ExecuteData*>(passData)); return false; }

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) { return Initialize(dev, buildData); }

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ static_cast<PassType>(CorePassType::CACAO) };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.Clean = Clean;
		return desc;
	}

	void Update(Device& dev, ExecuteData& passData, bool firstUpdate)
	{
		if (passData.RenderSize != Settings::RenderSize)
		{
			ZE_FFX_ENABLE();
			passData.RenderSize = Settings::RenderSize;

			if (!firstUpdate)
			{
				ZE_FFX_CHECK(ffxCacaoContextDestroy(&passData.Ctx), "Error destroying CACAO context!");
			}
			FfxCacaoContextDescription cacaoDesc = {};
			cacaoDesc.backendInterface = dev.GetFfxInterface();
			cacaoDesc.width = passData.RenderSize.X;
			cacaoDesc.height = passData.RenderSize.Y;
			cacaoDesc.useDownsampledSsao = false;
			ZE_FFX_THROW_FAILED(ffxCacaoContextCreate(&passData.Ctx, &cacaoDesc), "Error creating CACAO context!");
		}
	}

	void Clean(Device& dev, void* data) noexcept
	{
		ZE_FFX_ENABLE();
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		ZE_FFX_CHECK(ffxCacaoContextDestroy(&execData->Ctx), "Error destroying CACAO context!");
		delete execData;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData)
	{
		ExecuteData* passData = new ExecuteData;

		passData->Settings.generateNormals = false;
		Update(dev, *passData, true);

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_FFX_ENABLE();
		ZE_PERF_GUARD("CACAO");

		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

		ZE_DRAW_TAG_BEGIN(dev, cl, "CACAO", Pixel(0x89, 0xCF, 0xF0));

		data.Settings.temporalSupersamplingAngleOffset = Math::PI * Utils::SafeCast<float>(Settings::GetFrameIndex() % 3) / 3.0f;
		data.Settings.temporalSupersamplingRadiusOffset = 1.0f + (Utils::SafeCast<float>(Settings::GetFrameIndex() % 3) - 1.0f) / 30.0f;
		ZE_FFX_CHECK(ffxCacaoUpdateSettings(&data.Ctx, &data.Settings, false), "Error updating CACAO settings!");

		FfxCacaoDispatchDescription desc = {};
		desc.commandList = ffxGetCommandList(cl);
		//Resource::Generic depth, normal, ao;
		//desc.depthBuffer = ffxGetResource(renderData.Buffers, depth, ids.Depth, Resource::StateShaderResourceNonPS);
		//desc.normalBuffer = ffxGetResource(renderData.Buffers, normal, ids.Normal, Resource::StateShaderResourceNonPS);
		//desc.outputBuffer = ffxGetResource(renderData.Buffers, ao, ids.AO, Resource::StateUnorderedAccess);
		// Matrix data is not modified inside callbacks, missing const specifier in header
		desc.proj = reinterpret_cast<FfxCacaoMat4x4*>(&renderData.GraphData.Projection);
		desc.normalsToView = reinterpret_cast<FfxCacaoMat4x4*>(&renderData.DynamicData.ViewTps);
		// Custom way of loading normals is chosen so no need to perform any unpacking from SDK (custom callbacks provided)
		desc.normalUnpackMul = 1.0f;
		desc.normalUnpackAdd = 0.0f;
		ZE_FFX_THROW_FAILED(ffxCacaoContextDispatch(&data.Ctx, &desc), "Error performing CACAO!");

		ZE_DRAW_TAG_END(dev, cl);
	}
}
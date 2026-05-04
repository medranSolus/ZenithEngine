#include "GFX/Pipeline/RenderGraph.h"
#include "Data/Camera.h"
#include "Data/Transform.h"

#if _ZE_MODE_DEBUG || _ZE_MODE_DEV
#define ZE_SPLIT_SUBMISSIONS_DISABLED() if (!Settings::IsEnabledSplitRenderSubmissions())
#define ZE_SPLIT_SUBMISSIONS_BEGIN(list) do { if (Settings::IsEnabledSplitRenderSubmissions()) { ZE_LOG_RET_FAILED(list.Open(dev), "Failed to open split submission command list!"); } } while (false)
#define ZE_SPLIT_SUBMISSIONS_END(list, async) do { \
	if (Settings::IsEnabledSplitRenderSubmissions()) \
	{ \
		ZE_LOG_RET_FAILED(list.Close(dev), "Failed to close split submission command list!"); \
		async ? dev.ExecuteCompute(list) : dev.ExecuteMain(list); \
	} } while (false)
#else
#define ZE_SPLIT_SUBMISSIONS_DISABLED()
#define ZE_SPLIT_SUBMISSIONS_BEGIN(list)
#define ZE_SPLIT_SUBMISSIONS_END(list, async)
#endif

namespace ZE::GFX::Pipeline
{
	Status RenderGraph::PrepareFrameResources(Device& dev, SwapChain& swapChain) noexcept
	{
		execData.DynamicBuffer = &dynamicBuffers.Get();
		ZE_LOG_RET_FAILED(execData.DynamicBuffer->StartFrame(dev), "Failed to advance dynamic CBuffer into next frame!");
		auto exp = execData.DynamicBuffer->Alloc(dev, &execData.DynamicData, sizeof(RendererDynamicData));
		if (!exp)
		{
			ZE_CODE_ERROR(exp.error(), "Failed to allocate new frame's RendererDynamicData!");
			return exp.error();
		}
		ZE_LOG_RET_FAILED(execData.Buffers.SwapBackbuffer(dev, swapChain), "Failed to swap bacbuffers for framebuffer!");
		return {};
	}

	void RenderGraph::UnloadConfig() noexcept
	{
		passExecData.Clear();
		ffxInternalBuffers.Clear();
		execGroupCount = 0;
		passExecGroups = nullptr;
	}

	RenderGraph::~RenderGraph()
	{
		UnloadConfig();
		finalizationFlags = 0;
		FFX::DestroyInterface(ffxInterface);
	}

	Status RenderGraph::Execute(Graphics& gfx) noexcept
	{
		ZE_PERF_GUARD("Execute render graph");

		Device& dev = gfx.GetDevice();
		CommandList& mainList = gfx.GetMainList();
		CommandList& asyncList = asyncListChain.Get();
		if (asyncList.IsInitialized())
		{
			ZE_LOG_RET_FAILED(asyncList.Reset(dev), "Failed to reset async compute comand list!");
		}

		ZE_CODE_RET_FAILED(PrepareFrameResources(dev, gfx.GetSwapChain()));

		// TODO: Single threaded method only for now, but multiple threads possible as workers
		//       for a) passes in single pass group and then maybe for multiple pass groups at once
		//       but only if no synchronization issues could occur between pass states
		for (U32 i = 0; i < execGroupCount; ++i)
		{
			auto& mainGroup = passExecGroups[i].at(0);
			auto& asyncGroup = passExecGroups[i].at(1);

			if (mainGroup.PassGroupCount)
			{
				ZE_DRAW_TAG_BEGIN_MAIN(dev, "Main execution group, level " + std::to_string(i + 1), PixelVal::White);
				if (mainGroup.QueueWait)
				{
					ZE_LOG_RET_FAILED(dev.WaitMainFromCompute(mainGroup.WaitFence), "Failed to wait on GFX fence!");
				}

				ZE_SPLIT_SUBMISSIONS_DISABLED()
				{
					ZE_LOG_RET_FAILED(mainList.Open(dev), "Failed to open GFX command list!");
				}
				for (U32 j = 0; j < mainGroup.PassGroupCount; ++j)
				{
					auto& parallelGroup = mainGroup.PassGroups[j];
					if (parallelGroup.StartBarriers.size())
					{
						ZE_SPLIT_SUBMISSIONS_BEGIN(mainList);
						execData.Buffers.Barrier(mainList, parallelGroup.StartBarriers.data(), Utils::SafeCast<U32>(parallelGroup.StartBarriers.size()));
						ZE_SPLIT_SUBMISSIONS_END(mainList, false);
					}

					for (U32 k = 0; k < parallelGroup.PassCount; ++k)
					{
						ZE_SPLIT_SUBMISSIONS_BEGIN(mainList);
						bool run = false;
						ZE_EXPECT_RET_FAILED_CODE(run, parallelGroup.Passes[k].Exec(dev, mainList, execData, parallelGroup.Passes[k].Data));
						ZE_SPLIT_SUBMISSIONS_END(mainList, false);
					}
				}
				if (mainGroup.EndBarriers.size())
				{
					ZE_SPLIT_SUBMISSIONS_BEGIN(mainList);
					execData.Buffers.Barrier(mainList, mainGroup.EndBarriers.data(), Utils::SafeCast<U32>(mainGroup.EndBarriers.size()));
					ZE_SPLIT_SUBMISSIONS_END(mainList, false);
				}
				ZE_SPLIT_SUBMISSIONS_DISABLED()
				{
					ZE_LOG_RET_FAILED(mainList.Close(dev), "Failed to close GFX command list!");
					dev.ExecuteMain(mainList);
				}

				if (mainGroup.SignalFence)
				{
					ZE_EXPECT_RET_FAILED_CODE(*mainGroup.SignalFence, dev.SetMainFence());
				}
				ZE_DRAW_TAG_END_MAIN(dev);
			}

			if (asyncGroup.PassGroupCount)
			{
				ZE_DRAW_TAG_BEGIN_COMPUTE(dev, "Async execution group, level " + std::to_string(i + 1), PixelVal::White);
				if (asyncGroup.QueueWait)
				{
					ZE_LOG_RET_FAILED(dev.WaitComputeFromMain(asyncGroup.WaitFence), "Failed to wait on compute fence!");
				}

				ZE_SPLIT_SUBMISSIONS_DISABLED()
				{
					ZE_LOG_RET_FAILED(asyncList.Open(dev), "Failed to open compute command list!");
				}
				for (U32 j = 0; j < asyncGroup.PassGroupCount; ++j)
				{
					auto& parallelGroup = asyncGroup.PassGroups[j];
					if (parallelGroup.StartBarriers.size())
					{
						ZE_SPLIT_SUBMISSIONS_BEGIN(asyncList);
						execData.Buffers.Barrier(asyncList, parallelGroup.StartBarriers.data(), Utils::SafeCast<U32>(parallelGroup.StartBarriers.size()));
						ZE_SPLIT_SUBMISSIONS_END(asyncList, true);
					}

					for (U32 k = 0; k < parallelGroup.PassCount; ++k)
					{
						ZE_SPLIT_SUBMISSIONS_BEGIN(asyncList);
						bool run = false;
						ZE_EXPECT_RET_FAILED_CODE(run, parallelGroup.Passes[k].Exec(dev, asyncList, execData, parallelGroup.Passes[k].Data));
						ZE_SPLIT_SUBMISSIONS_END(asyncList, true);
					}
				}
				if (asyncGroup.EndBarriers.size())
				{
					ZE_SPLIT_SUBMISSIONS_BEGIN(asyncList);
					execData.Buffers.Barrier(asyncList, asyncGroup.EndBarriers.data(), Utils::SafeCast<U32>(asyncGroup.EndBarriers.size()));
					ZE_SPLIT_SUBMISSIONS_END(asyncList, true);
				}
				ZE_SPLIT_SUBMISSIONS_DISABLED()
				{
					ZE_LOG_RET_FAILED(asyncList.Close(dev), "Failed to close compute command list!");
					dev.ExecuteCompute(asyncList);
				}

				if (asyncGroup.SignalFence)
				{
					ZE_EXPECT_RET_FAILED_CODE(*asyncGroup.SignalFence, dev.SetComputeFence());
				}
				ZE_DRAW_TAG_END_COMPUTE(dev);
			}
		}
		return {};
	}

	void RenderGraph::SetCamera(EID camera) noexcept
	{
		ZE_VALID_EID(execData.GraphData.CurrentCamera);
		execData.GraphData.CurrentCamera = camera;
	}

	void RenderGraph::UpdateFrameData(Device& dev) noexcept
	{
		ZE_VALID_EID(execData.GraphData.CurrentCamera);
		ZE_ASSERT((Settings::Data.all_of<Data::TransformGlobal, Data::Camera>(execData.GraphData.CurrentCamera)),
			"Current camera does not have all required components!");

		// Copy previous camera info
		execData.GraphData.PrevViewTps = execData.DynamicData.ViewTps;
		execData.GraphData.PrevProjection = execData.GraphData.Projection;

		auto& currentCamera = Settings::Data.get<Data::Camera>(execData.GraphData.CurrentCamera);
		const auto& transform = Settings::Data.get<Data::Transform>(execData.GraphData.CurrentCamera); // TODO: Change into TransformGlobal later

		// Setup shader dynamic data
		execData.DynamicData.CameraPos = transform.Position;
		execData.DynamicData.NearClip = currentCamera.Projection.NearClip;
		const Matrix view = Math::XMMatrixLookToLH(Math::XMLoadFloat3(&transform.Position),
			Math::XMLoadFloat3(&currentCamera.EyeDirection),
			Math::XMLoadFloat3(&currentCamera.UpVector));
		Math::XMStoreFloat4x4(&execData.DynamicData.ViewTps, Math::XMMatrixTranspose(view));

		if (Settings::ApplyJitter())
		{
			CalculateJitter(dev, execData.GraphData.JitterIndex, currentCamera.Projection.JitterX,
				currentCamera.Projection.JitterY, Settings::RenderSize, Settings::DisplaySize, Settings::Upscaler);
			execData.DynamicData.JitterPrev = execData.DynamicData.JitterCurrent;
			execData.DynamicData.JitterCurrent = { currentCamera.Projection.JitterX, currentCamera.Projection.JitterY };
		}
		else
			execData.DynamicData.JitterPrev = execData.DynamicData.JitterCurrent = { 0.0f, 0.0f };

		Matrix projection = Data::GetProjectionMatrix(currentCamera.Projection);
		Math::XMStoreFloat4x4(&execData.GraphData.Projection, projection);

		const Matrix viewProjection = view * projection;
		Math::XMStoreFloat4x4(&execData.DynamicData.ViewProjectionTps, Math::XMMatrixTranspose(viewProjection));
		Math::XMStoreFloat4x4(&execData.DynamicData.ViewProjectionInverseTps, Math::XMMatrixTranspose(Math::XMMatrixInverse(nullptr, viewProjection)));
	}
}
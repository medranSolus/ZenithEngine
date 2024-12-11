#include "GFX/Pipeline/RenderGraph.h"
#include "Data/Camera.h"
#include "Data/Transform.h"

namespace ZE::GFX::Pipeline
{
	void RenderGraph::UnloadConfig(Device& dev) noexcept
	{
		passExecData.Transform([&dev](const auto& passData)
			{
				if (passData.first)
				{
					ZE_ASSERT(passData.second, "Clean function should always be present when pass exec data is not empty!");
					GpuSyncStatus status = { true, true, true };
					passData.second(dev, passData.first, status);
				}
			});
		passExecData.Clear();

		ffxInternalBuffers.Clear();
		execGroupCount = 0;
		passExecGroups = nullptr;
	}

	BuildResult RenderGraph::Execute(Graphics& gfx, Data::AssetsStreamer& assets, RenderGraphBuilder* builder)
	{
		Device& dev = gfx.GetDevice();
		CommandList& asyncList = asyncListChain.Get();
		if (asyncList.IsInitialized())
			asyncList.Reset(dev);
		CommandList& mainList = gfx.GetMainList();

		if (builder)
		{
			BuildResult result = builder->UpdatePassConfiguration(dev, assets, *this);
			if (result != BuildResult::Success)
				return result;
			assets.GetDisk().StartUploadGPU(true); // TODO: Better for it to have some kind of token or ID into wait or anything, maybe with map of events, something like that
		}

		ZE_PERF_START("Update upload data status");
		const bool gpuWorkPending = assets.GetDisk().IsGPUWorkPending();
		if (gpuWorkPending)
			mainList.Open(dev);

		[[maybe_unused]] bool status = assets.GetDisk().WaitForUploadGPU(dev, mainList);
		ZE_ASSERT(status, "Error uploading engine GPU data!");

		if (gpuWorkPending)
		{
			mainList.Close(dev);
			// If any async passes has been processed then sync for initialization of resources
			if (asyncList.IsInitialized())
			{
				dev.ExecuteMain(mainList);
				dev.WaitComputeFromMain(dev.SetMainFence());
			}
		}
		ZE_PERF_STOP();

		execData.DynamicBuffer = &dynamicBuffers.Get();
		execData.DynamicBuffer->StartFrame(dev);
		execData.DynamicBuffer->Alloc(dev, &execData.DynamicData, sizeof(RendererDynamicData));
		execData.Buffers.SwapBackbuffer(dev, gfx.GetSwapChain());

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
					dev.WaitMainFromCompute(mainGroup.WaitFence);

				mainList.Open(dev);
				for (U32 j = 0; j < mainGroup.PassGroupCount; ++j)
				{
					auto& parallelGroup = mainGroup.PassGroups[j];
					if (parallelGroup.StartBarriers.size())
						execData.Buffers.Barrier(mainList, parallelGroup.StartBarriers.data(), Utils::SafeCast<U32>(parallelGroup.StartBarriers.size()));

					for (U32 k = 0; k < parallelGroup.PassCount; ++k)
						parallelGroup.Passes[k].Exec(dev, mainList, execData, parallelGroup.Passes[k].Data);
				}
				if (mainGroup.EndBarriers.size())
					execData.Buffers.Barrier(mainList, mainGroup.EndBarriers.data(), Utils::SafeCast<U32>(mainGroup.EndBarriers.size()));
				mainList.Close(dev);
				dev.ExecuteMain(mainList);

				if (mainGroup.SignalFence)
					*mainGroup.SignalFence = dev.SetMainFence();
				ZE_DRAW_TAG_END_MAIN(dev);
			}

			if (asyncGroup.PassGroupCount)
			{
				ZE_DRAW_TAG_BEGIN_COMPUTE(dev, "Async execution group, level " + std::to_string(i + 1), PixelVal::White);
				if (asyncGroup.QueueWait)
					dev.WaitComputeFromMain(asyncGroup.WaitFence);

				asyncList.Open(dev);
				for (U32 j = 0; j < asyncGroup.PassGroupCount; ++j)
				{
					auto& parallelGroup = asyncGroup.PassGroups[j];
					if (parallelGroup.StartBarriers.size())
						execData.Buffers.Barrier(asyncList, parallelGroup.StartBarriers.data(), Utils::SafeCast<U32>(parallelGroup.StartBarriers.size()));

					for (U32 k = 0; k < parallelGroup.PassCount; ++k)
						parallelGroup.Passes[k].Exec(dev, asyncList, execData, parallelGroup.Passes[k].Data);
				}
				if (asyncGroup.EndBarriers.size())
					execData.Buffers.Barrier(asyncList, asyncGroup.EndBarriers.data(), Utils::SafeCast<U32>(asyncGroup.EndBarriers.size()));
				asyncList.Close(dev);
				dev.ExecuteCompute(asyncList);

				if (asyncGroup.SignalFence)
					*asyncGroup.SignalFence = dev.SetComputeFence();
				ZE_DRAW_TAG_END_COMPUTE(dev);
			}
		}
		return BuildResult::Success;
	}

	void RenderGraph::SetCamera(EID camera)
	{
		ZE_VALID_EID(execData.GraphData.CurrentCamera);
		execData.GraphData.CurrentCamera = camera;
	}

	void RenderGraph::UpdateFrameData(Device& dev)
	{
		ZE_VALID_EID(execData.GraphData.CurrentCamera);
		ZE_ASSERT((Settings::Data.all_of<Data::TransformGlobal, Data::Camera>(execData.GraphData.CurrentCamera)),
			"Current camera does not have all required components!");

		auto& currentCamera = Settings::Data.get<Data::Camera>(execData.GraphData.CurrentCamera);
		const auto& transform = Settings::Data.get<Data::Transform>(execData.GraphData.CurrentCamera); // TODO: Change into TransformGlobal later

		// Setup shader dynamic data
		execData.DynamicData.CameraPos = transform.Position;
		execData.DynamicData.NearClip = currentCamera.Projection.NearClip;
		const Matrix view = Math::XMMatrixLookToLH(Math::XMLoadFloat3(&transform.Position),
			Math::XMLoadFloat3(&currentCamera.EyeDirection),
			Math::XMLoadFloat3(&currentCamera.UpVector));
		Math::XMStoreFloat4x4(&execData.DynamicData.ViewTps, Math::XMMatrixTranspose(view));

		if (Settings::ComputeMotionVectors())
			execData.GraphData.PrevViewProjectionTps = execData.DynamicData.ViewProjectionTps;

		if (Settings::ApplyJitter())
		{
			CalculateJitter(execData.GraphData.JitterIndex, currentCamera.Projection.JitterX, currentCamera.Projection.JitterY, Settings::RenderSize, Settings::Upscaler);
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

	void RenderGraph::ShowDebugUI() noexcept
	{
		ImGui::BeginChild("##render_graph", { 100.0f, 100.0f }, ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysVerticalScrollbar);

		ImGui::EndChild();
	}

	void RenderGraph::Free(Device& dev) noexcept
	{
		UnloadConfig(dev);

		finalizationFlags = 0;
		FFX::DestroyInterface(ffxInterface);
		asyncListChain.Exec([&dev](CommandList& x) { x.Free(dev); });
		dynamicBuffers.Exec([&dev](Resource::DynamicCBuffer& x) { x.Free(dev); });
		execData.Buffers.Free(dev);
		execData.Bindings.Free(dev);
		execData.SettingsBuffer.Free(dev);
	}
}
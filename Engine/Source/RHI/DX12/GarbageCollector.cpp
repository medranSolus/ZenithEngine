#include "RHI/DX12/GarbageCollector.h"

namespace ZE::RHI::DX12
{
	void GarbageCollector::CollectFrame(FrameBucket& frame, Device& dev) noexcept
	{
		auto devRes = frame.DevResources.find(dev.GetDevice());
		if (devRes != frame.DevResources.end())
		{
			for (auto& desc : devRes->second.Descriptors)
				dev.FreeDescs(desc);
			for (auto& res : devRes->second.Resources)
			{
				switch (res.first)
				{
				default:
					ZE_ENUM_UNHANDLED();
				case ResourceType::Unknown:
					break;
				case ResourceType::Buffer:
					dev.FreeBuffer(res.second);
					break;
				case ResourceType::DynamicBuffer:
					dev.FreeDynamicBuffer(res.second);
					break;
				case ResourceType::Texture:
					dev.FreeTexture(res.second);
					break;
				}
			}
			frame.DevResources.erase(devRes);
		}
	}

	IDevice* GarbageCollector::MarkInactive(AllocHandle handle) noexcept
	{
		ZE_ASSERT(activeDeviceResources.contains(handle), "Resource not yet registered in garbage collector!");

		IDevice* dev = activeDeviceResources.at(handle);
		activeDeviceResources.erase(handle);
		return dev;
	}

	void GarbageCollector::AdvanceFrame(Device& dev) noexcept
	{
		CollectFrame(frames.back(), dev);
		if (lastFrameIndex < Settings::GetFrameIndex())
		{
			lastFrameIndex = Settings::GetFrameIndex();
			ZE_ASSERT(frames.size(), "No frame queue!");
			ZE_ASSERT(!frames.back().DevResources.size(), "Not all resources were collected for the frame!");

			frames.pop_back();
			while (frames.size() <= Settings::GetBackbufferCount())
				frames.emplace_front();
		}
	}

	void GarbageCollector::Flush(Device& dev) noexcept
	{
		ZE_ASSERT(activeDeviceResources.size() == 0, "At this point all resources should be already registered for deletion!");
		for (auto& frame : frames)
		{
			CollectFrame(frame, dev);
			frame.D3dObjects.clear();
		}
	}
}
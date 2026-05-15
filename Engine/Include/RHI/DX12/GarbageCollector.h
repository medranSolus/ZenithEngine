#pragma once
#include "Device.h"

namespace ZE::RHI::DX12
{
	class GarbageCollector final
	{
		enum class ResourceType : U8
		{
			Unknown, Buffer, DynamicBuffer, Texture,
		};

		struct DeviceResources
		{
			std::vector<DescriptorInfo> Descriptors;
			std::vector<std::pair<ResourceType, ResourceInfo>> Resources;
		};

		struct FrameBucket
		{
			std::vector<DX::ComPtr<IUnknown>> D3dObjects;
			std::map<IDevice*, DeviceResources> DevResources;
		};

		std::map<AllocHandle, IDevice*> activeDeviceResources;
		std::deque<FrameBucket> frames;
		U64 lastFrameIndex = 0;

		void CollectFrame(FrameBucket& frame, Device& dev) noexcept;

		GarbageCollector() noexcept : frames(Settings::GetBackbufferCount() + 1) {}
		ZE_CLASS_DEFAULT(GarbageCollector);

	public:
		~GarbageCollector() = default;

		static GarbageCollector& Get() noexcept { static GarbageCollector instance; return instance; }

		// Register D3D object for deletion postponed by number of frames in flight
		void Register(DX::ComPtr<IUnknown> object) noexcept { frames.front().D3dObjects.emplace_back(std::move(object)); }
		void Register(IDevice* dev, DescriptorInfo&& descs) noexcept { frames.front().DevResources[dev].Descriptors.emplace_back(std::move(descs)); }
		void RegisterBuffer(IDevice* dev, ResourceInfo&& res) noexcept { frames.front().DevResources[dev].Resources.emplace_back(ResourceType::Buffer, std::move(res)); }
		void RegisterDynamicBuffer(IDevice* dev, ResourceInfo&& res) noexcept { frames.front().DevResources[dev].Resources.emplace_back(ResourceType::DynamicBuffer, std::move(res)); }
		void RegisterTexture(IDevice* dev, ResourceInfo&& res) noexcept { frames.front().DevResources[dev].Resources.emplace_back(ResourceType::Texture, std::move(res)); }

		void MarkActive(Device& dev, AllocHandle handle) noexcept { ZE_ASSERT(!activeDeviceResources.contains(handle), "Resource already registered in garbage collector!"); activeDeviceResources[handle] = dev.GetDevice(); }

		IDevice* MarkInactive(AllocHandle handle) noexcept;
		void AdvanceFrame(Device& dev) noexcept;
		void Flush(Device& dev) noexcept;
	};
}
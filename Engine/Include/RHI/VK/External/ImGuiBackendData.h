#pragma once
#include "GFX/CommandList.h"

namespace ZE::RHI::VK::External
{
	class ImGuiBackendData final
	{
		struct AllocData
		{
			VkRenderPass RenderPass;
			VkFramebuffer Framebuffer;
			VkDescriptorPool DescPool;
			Device* srcDev = nullptr;
		};
		std::unique_ptr<AllocData> data;

	public:
		ImGuiBackendData() = default;
		ZE_CLASS_MOVE(ImGuiBackendData);
		~ImGuiBackendData();

		static Expected<ImGuiBackendData> Create(GFX::Device& dev, PixelFormat outputFormat) noexcept;

		static void RecreateFonts() noexcept;
		static void NewFrame() noexcept;

		void RunRender(GFX::CommandList& cl) const noexcept;
	};
}
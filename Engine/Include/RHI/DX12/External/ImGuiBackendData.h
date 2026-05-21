#pragma once
#include "GFX/CommandList.h"

namespace ZE::RHI::DX12::External
{
	class ImGuiBackendData final
	{
		struct AllocData
		{
			std::unordered_map<U64, DescriptorInfo> AllocatedDescs;
			Device* SrcDev = nullptr;
		};
		std::unique_ptr<AllocData> data;

	public:
		ImGuiBackendData() = default;
		ZE_CLASS_MOVE(ImGuiBackendData);
		~ImGuiBackendData();

		static Expected<ImGuiBackendData> Create(GFX::Device& dev, PixelFormat outputFormat) noexcept;

		static void RecreateFonts() noexcept {}
		static void NewFrame() noexcept;

		void RunRender(GFX::CommandList& cl) const noexcept;
	};
}
#pragma once
#include "GFX/CommandList.h"

namespace ZE::RHI::DX11::External
{
	class ImGuiBackendData final
	{
		Move<bool, false> created;

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
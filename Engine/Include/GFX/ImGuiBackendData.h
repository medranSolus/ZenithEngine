#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/ImGuiBackendData.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/ImGuiBackendData.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/ImGuiBackendData.h"
#endif

namespace ZE::GFX
{
	// Backend data used by ImGui during rendering
	class ImGuiBackendData final
	{
		ZE_RHI_BACKEND(ImGuiBackendData);

	public:
		ImGuiBackendData() = default;
		ZE_CLASS_MOVE(ImGuiBackendData);
		~ImGuiBackendData() = default;

		static Expected<ImGuiBackendData> Create(Device& dev, PixelFormat outputFormat) noexcept { ZE_RHI_BACKEND_CREATE(ImGuiBackendData, dev, outputFormat); }
		ZE_RHI_BACKEND_GET(ImGuiBackendData);

		// Main Gfx API

		static constexpr void NewFrame() noexcept { ZE_RHI_BACKEND_CALL_STATIC(ImGuiBackendData, NewFrame); }
		static constexpr void RecreateFonts() noexcept { ZE_RHI_BACKEND_CALL_STATIC(ImGuiBackendData, RecreateFonts); }

		constexpr void RunRender(CommandList& cl) const noexcept { ZE_RHI_BACKEND_CALL(RunRender, cl); }
	};
}
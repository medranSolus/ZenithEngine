#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/External/ImGuiBackendData.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/External/ImGuiBackendData.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/External/ImGuiBackendData.h"
#endif

namespace ZE::GFX::External
{
	// Backend data used by ImGui during rendering
	class ImGuiBackendData final
	{
		ZE_RHI_BACKEND(External::ImGuiBackendData);

	public:
		ImGuiBackendData() = default;
		ZE_CLASS_MOVE(External::ImGuiBackendData);
		~ImGuiBackendData() = default;

		static Expected<ImGuiBackendData> Create(Device& dev, PixelFormat outputFormat) noexcept { ZE_RHI_BACKEND_CREATE(External::ImGuiBackendData, dev, outputFormat); }
		ZE_RHI_BACKEND_GET(External::ImGuiBackendData);
		// Main Gfx API

		static constexpr void NewFrame() noexcept { ZE_RHI_BACKEND_CALL_STATIC(External::ImGuiBackendData, NewFrame); }
		static constexpr void RecreateFonts() noexcept { ZE_RHI_BACKEND_CALL_STATIC(External::ImGuiBackendData, RecreateFonts); }

		constexpr void RunRender(CommandList& cl) const noexcept { ZE_RHI_BACKEND_CALL(RunRender, cl); }
	};
}
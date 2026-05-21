#include "RHI/DX11/External/ImGuiBackendData.h"
ZE_WARNING_PUSH
#include "backends/imgui_impl_dx11.h"
ZE_WARNING_POP

namespace ZE::RHI::DX11::External
{
	void ImGuiBackendData::Destroy() noexcept
	{
		if (created)
			ImGui_ImplDX11_Shutdown();
	}

	void ImGuiBackendData::MoveFrom(ImGuiBackendData&& backend) noexcept
	{
		created = std::exchange(backend.created, false);
	}

	Expected<ImGuiBackendData> ImGuiBackendData::Create(GFX::Device& dev, PixelFormat outputFormat) noexcept
	{
		ImGuiBackendData backend;

		ImGui_ImplDX11_Init(dev.Get().dx11.GetDevice(), dev.Get().dx11.GetMainContext());
		backend.created = true;

		return backend;
	}

	void ImGuiBackendData::NewFrame() noexcept
	{
		ImGui_ImplDX11_NewFrame();
	}

	void ImGuiBackendData::RunRender(GFX::CommandList& cl) const noexcept
	{
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
}
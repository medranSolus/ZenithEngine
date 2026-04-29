#pragma once
#include "GFX/CommandList.h"

namespace ZE::RHI::DX11
{
	class ImGuiBackendData final
	{
		bool created = false;

		void Destroy() noexcept;
		void MoveFrom(ImGuiBackendData&& backend) noexcept;

	public:
		ImGuiBackendData() = default;
		ZE_CLASS_NO_COPY(ImGuiBackendData);
		ImGuiBackendData(ImGuiBackendData&& backend) noexcept { MoveFrom(std::move(backend)); }
		ImGuiBackendData& operator=(ImGuiBackendData&& backend) noexcept { Destroy(); MoveFrom(std::move(backend)); return *this; }
		~ImGuiBackendData() { Destroy(); }

		static Expected<ImGuiBackendData> Create(GFX::Device& dev, PixelFormat outputFormat) noexcept;

		static void RecreateFonts() noexcept {}
		static void NewFrame() noexcept;

		void RunRender(GFX::CommandList& cl) const noexcept;
	};
}
#pragma once
#include "ICamera.h"
#include "WinAPI/Window.h"
#include "GFX/Pipeline/RenderGraph.h"
#include <unordered_map>

namespace Camera
{
	class CameraPool final
	{
		static constexpr float MAX_MOVE_SPEED = 5.0f;

		float moveSpeed = 0.1f;
		float rollSpeed = 0.01f;
		float rotateSpeed = 1.5f;
		bool cameraChanged = true;
		std::string active;
		std::unordered_map<std::string, std::unique_ptr<ICamera>> cameras;

	public:
		CameraPool(std::unique_ptr<ICamera>&& camera) noexcept
			: active(camera->GetName()) { cameras.emplace(active, std::move(camera)); }
		CameraPool(CameraPool&&) = default;
		CameraPool(const CameraPool&) = default;
		CameraPool& operator=(CameraPool&&) = default;
		CameraPool& operator=(const CameraPool&) = default;
		~CameraPool() = default;

		constexpr bool CameraChanged() noexcept { bool changed = cameraChanged; cameraChanged = false; return changed; }
		ICamera& GetCamera() noexcept { return *cameras.at(active); }
		void SetOutline() noexcept { cameras.at(active)->SetOutline(); }
		void DisableOutline() noexcept { cameras.at(active)->DisableOutline(); }

		void ProcessInput(WinAPI::Window& window) noexcept;
		bool AddCamera(std::unique_ptr<ICamera>&& camera) noexcept;
		bool DeleteCamera(const std::string& name) noexcept;
		bool Accept(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, GFX::Probe::BaseProbe& probe) noexcept;
		void Submit(U64 channelFilter) noexcept;
	};
}
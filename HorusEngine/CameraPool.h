#pragma once
#include "ICamera.h"
#include "Window.h"
#include "RenderGraph.h"
#include <map>

namespace Camera
{
	class CameraPool
	{
		static constexpr float MAX_MOVE_SPEED = 5.0f;

		float moveSpeed = 0.1f;
		float rollSpeed = 0.01;
		float rotateSpeed = 1.5f;
		bool cameraChanged = true;
		std::string active;
		std::map<std::string, std::unique_ptr<ICamera>> cameras;

	public:
		inline CameraPool(std::unique_ptr<ICamera> camera) noexcept : active(camera->GetName()) { cameras.emplace(active, std::move(camera)); }
		virtual ~CameraPool() = default;

		inline ICamera& GetCamera() noexcept { return *cameras.at(active); }

		constexpr bool CameraChanged() noexcept { bool changed = cameraChanged; cameraChanged = false; return changed; }
		inline void SetOutline() noexcept { cameras.at(active)->SetOutline(); }
		inline void DisableOutline() noexcept { cameras.at(active)->DisableOutline(); }

		void ProcessInput(WinAPI::Window& window) noexcept;
		bool AddCamera(std::unique_ptr<ICamera> camera) noexcept;
		bool DeleteCamera(const std::string& name) noexcept;
		bool Accept(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, GFX::Probe::BaseProbe& probe) noexcept;
		void Submit(uint64_t channelFilter) noexcept;
	};
}
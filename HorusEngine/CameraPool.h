#pragma once
#include "ICamera.h"
#include "Window.h"
#include <map>

namespace Camera
{
	class CameraPool : public GFX::Pipeline::IRenderable
	{
		static constexpr float MAX_MOVE_SPEED = 5.0f;

		float moveSpeed = 0.1f;
		float rollSpeed = 0.01;
		float rotateSpeed = 2.0f;
		std::string active;
		std::map<std::string, std::unique_ptr<ICamera>> cameras;

	public:
		inline CameraPool(std::unique_ptr<ICamera> camera) noexcept : active(camera->GetName()) { cameras.emplace(active, std::move(camera)); }
		virtual ~CameraPool() = default;

		inline ICamera& GetCamera() noexcept { return *cameras.at(active); }

		inline void SetOutline() noexcept override { cameras.at(active)->SetOutline(); }
		inline void DisableOutline() noexcept override { cameras.at(active)->DisableOutline(); }

		void ProcessInput(WinAPI::Window& window) noexcept;
		bool AddCamera(std::unique_ptr<ICamera> camera) noexcept;
		bool DeleteCamera(const std::string& name) noexcept;
		void Accept(GFX::Graphics& gfx, GFX::Probe::BaseProbe& probe) noexcept override;
		void Submit(uint64_t channelFilter) noexcept override;
	};
}
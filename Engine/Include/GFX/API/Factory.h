#pragma once
#include "Backend.h"
#include "Window/MainWindow.h"
#include "GFX/Context.h"
#include "GFX/Device.h"
#include "GFX/GPerf.h"
#include "GFX/SwapChain.h"

namespace ZE
{
	class Engine;
}
namespace ZE::GFX::API
{
	// Factory used for creation of API specific objects
	class Factory final
	{
		friend class Engine;

		Backend currentApi;

		void InitGui(const Device& dev, const Context& ctx) const noexcept;
		void DisableGui() const noexcept;
		void StartGuiFrame() const noexcept;
		void EndGuiFrame() const noexcept;

	public:
		constexpr Factory(Backend type) noexcept : currentApi(type) {}
		Factory(Factory&&) = delete;
		Factory(const Factory&) = delete;
		Factory& operator=(Factory&&) = delete;
		Factory& operator=(const Factory&) = delete;
		~Factory() = default;

		//constexpr void ChangeBackend(Backend type) noexcept { currentApi = type; }

		Context* MakeContext(Device& dev, bool deffered);
		Device* MakeDevice();
		GPerf* MakeGpuPerf(Device& dev);
		SwapChain* MakeSwapChain(const Window::MainWindow& window, Device& dev);
	};
}
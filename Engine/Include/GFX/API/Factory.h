#pragma once
#include "Backend.h"
#include "Window/MainWindow.h"
#include "GFX/CommandList.h"
#include "GFX/DeferredContext.h"
#include "GFX/Device.h"
#include "GFX/GPerf.h"
#include "GFX/MainContext.h"
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

		void InitGui(const Device& dev, const MainContext& ctx) const noexcept;
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

		[[nodiscard]] CommandList* MakeCommandList() const;
		[[nodiscard]] Device* MakeDevice() const;
		[[nodiscard]] GPerf* MakeGpuPerf(Device& dev) const;
		[[nodiscard]] DeferredContext* MakeDeferredContext(Device& dev) const;
		[[nodiscard]] MainContext* MakeMainContext(Device& dev) const;
		[[nodiscard]] SwapChain* MakeSwapChain(const Window::MainWindow& window, Device& dev) const;
	};
}
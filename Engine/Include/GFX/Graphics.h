#pragma once
#include "Settings.h"

namespace ZE::GFX
{
	class Graphics final
	{
		Device* device = nullptr;
		MainContext* mainCtx = nullptr;
		SwapChain* swapChain = nullptr;
		bool guiEnabled = true;

	public:
		Graphics() = default;
		Graphics(Graphics&&) = delete;
		Graphics(const Graphics&) = delete;
		Graphics& operator=(Graphics&&) = delete;
		Graphics& operator=(const Graphics&) = delete;
		~Graphics();

		constexpr Device& GetDevice() noexcept { return *device; }
		constexpr MainContext& GetMainContext() noexcept { return *mainCtx; }

		void Init(const Window::MainWindow& window);
		void Present() const;
	};
}
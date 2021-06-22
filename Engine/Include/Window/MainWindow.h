#pragma once
#include "Keyboard.h"
#include "Mouse.h"
#include "GFX/Graphics.h"

namespace ZE::Window
{
	class BaseWindow
	{
	protected:
		Keyboard keyboard;
		Mouse mouse;
		GFX::Graphics graphics;
		bool cursorEnabled = true;

		virtual void ShowCursor() noexcept = 0;
		virtual void HideCursor() noexcept = 0;
		virtual void FreeCursor() noexcept = 0;
		virtual void TrapCursor() noexcept = 0;

	public:
		BaseWindow(const char* name, U32 width = 0, U32 height = 0) noexcept {}
		BaseWindow(BaseWindow&&) = delete;
		BaseWindow(const BaseWindow&) = delete;
		BaseWindow& operator=(BaseWindow&&) = delete;
		BaseWindow& operator=(const BaseWindow&) = delete;
		virtual ~BaseWindow() = default;

		constexpr Keyboard& Keyboard() noexcept { return keyboard; }
		constexpr Mouse& Mouse() noexcept { return mouse; }
		constexpr GFX::Graphics& Gfx() noexcept { return graphics; }
		constexpr bool IsCursorEnabled() const noexcept { return cursorEnabled; }
		void SwitchCursor() noexcept { cursorEnabled ? DisableCursor() : EnableCursor(); }

		void EnableCursor() noexcept;
		void DisableCursor() noexcept;

		virtual std::pair<bool, int> ProcessMessage() noexcept = 0;
		virtual void SetTitle(const std::string& title) = 0;
	};
}

#if _ZE_PLATFORM == _ZE_PLATFORM_WINDOWS
#include "Platform/WindowWinAPI.h"
namespace ZE::Window
{
	typedef WinAPI::WindowWinAPI MainWindow;
}
#else
#error Missing window platform specyfic implementation!
#endif
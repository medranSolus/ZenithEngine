#pragma once
#include "Keyboard.h"
#include "Mouse.h"
#include "imgui.h"
#include <bitset>

namespace ZE::Window
{
	// Platform agnostic window logic
	class BaseWindow
	{
	protected:
		Keyboard keyboard;
		Mouse mouse;
		// 0: cursor, 1: fullscreen
		std::bitset<2> flags;

		virtual void ShowCursor() const noexcept = 0;
		virtual void HideCursor() const noexcept = 0;
		virtual void FreeCursor() const noexcept = 0;
		virtual void TrapCursor() const noexcept = 0;
		virtual void EnterFullscreen() noexcept = 0;
		virtual void LeaveFullscreen() noexcept = 0;

	public:
		BaseWindow() noexcept { flags[0] = true; flags[1] = false; }
		ZE_CLASS_DELETE(BaseWindow);
		virtual ~BaseWindow() = default;

		constexpr Keyboard& Keyboard() noexcept { return keyboard; }
		constexpr Mouse& Mouse() noexcept { return mouse; }
		constexpr bool IsCursorEnabled() const noexcept { return flags[0]; }
		constexpr bool IsFullscreen() const noexcept { return flags[1]; }
		void SwitchCursor() noexcept { IsCursorEnabled() ? DisableCursor() : EnableCursor(); }

		void EnableCursor() noexcept;
		void DisableCursor() noexcept;
		void SwitchFullscreen() noexcept;

		virtual U32 GetWidth() const noexcept = 0;
		virtual U32 GetHeight() const noexcept = 0;
		virtual void Init(std::string_view name, U32 width = 0, U32 height = 0) = 0;
		virtual std::pair<bool, int> ProcessMessage() noexcept = 0;
		virtual void SetTitle(std::string_view title) = 0;
		virtual void NewGuiFrame() const noexcept = 0;
	};
}
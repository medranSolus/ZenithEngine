#pragma once
#include "Keyboard.h"
#include "Mouse.h"
#include "GFX/Graphics.h"

namespace ZE::WinAPI
{
	class Window final
	{
		class WindowClass final
		{
			static constexpr const char* WND_CLASS_NAME = "ZE_WND";

			HINSTANCE hInstance;

		public:
			WindowClass() noexcept;
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator=(const WindowClass&) = delete;
			~WindowClass() { UnregisterClass(WND_CLASS_NAME, hInstance); }

			static constexpr const char* GetName() noexcept { return WND_CLASS_NAME; }

			constexpr HINSTANCE GetInstance() const noexcept { return hInstance; }
		};

		static inline WindowClass wndClass;

		Keyboard keyboard;
		Mouse mouse;
		GFX::Graphics graphics;
		std::vector<U8> rawBuffer;
		bool cursorEnabled = true;
		HWND hWnd;
		UINT dpi;

		void ShowCursor() noexcept { while (::ShowCursor(TRUE) < 0); }
		void HideCursor() noexcept { while (::ShowCursor(FALSE) >= 0); }
		void FreeCursor() noexcept { ClipCursor(nullptr); }

		static LRESULT WINAPI HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		static LRESULT WINAPI HandleMsgStub(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

		LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		void TrapCursor() noexcept;

	public:
		Window(const char* name, U32 width = 0, U32 height = 0);
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		~Window();

		constexpr Keyboard& Keyboard() noexcept { return keyboard; }
		constexpr Mouse& Mouse() noexcept { return mouse; }
		constexpr bool IsCursorEnabled() const noexcept { return cursorEnabled; }
		constexpr GFX::Graphics& Gfx() noexcept { return graphics; }
		void SwitchCursor() noexcept { cursorEnabled ? DisableCursor() : EnableCursor(); }

		static std::pair<bool, int> ProcessMessage() noexcept;

		void SetTitle(const std::string& title);
		void EnableCursor() noexcept;
		void DisableCursor() noexcept;
	};
}
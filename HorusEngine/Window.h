#pragma once
#include "WinApiException.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Graphics.h"

namespace WinAPI
{
	class Window
	{
		class WindowClass final
		{
			static constexpr const char* WND_CLASS_NAME = "horusEngineWindow";

			HINSTANCE hInstance;

		public:
			WindowClass() noexcept;
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator=(const WindowClass&) = delete;
			inline ~WindowClass() { UnregisterClass(WND_CLASS_NAME, hInstance); }

			static constexpr const char* GetName() noexcept { return WND_CLASS_NAME; }

			constexpr HINSTANCE GetInstance() const noexcept { return hInstance; }
		};

		static WindowClass wndClass;

		unsigned int wndWidth;
		unsigned int wndHeight;
		bool cursorEnabled = true;
		HWND hWnd;
		Keyboard keyboard;
		Mouse mouse;
		std::vector<char> rawBuffer;
		std::unique_ptr<GFX::Graphics> graphics = nullptr;

		inline void ShowCursor() noexcept { while (::ShowCursor(TRUE) < 0); }
		inline void HideCursor() noexcept { while (::ShowCursor(FALSE) >= 0); }
		inline void FreeCursor() noexcept { ClipCursor(nullptr); }

		static LRESULT WINAPI HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		static LRESULT WINAPI HandleMsgStub(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

		LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		void TrapCursor() noexcept;

	public:
		Window(unsigned int width, unsigned int height, const char* name);
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		~Window();

		constexpr Keyboard& Keyboard() noexcept { return keyboard; }
		constexpr Mouse& Mouse() noexcept { return mouse; }
		constexpr bool IsCursorEnabled() const noexcept { return cursorEnabled; }
		inline void SwitchCursor() noexcept { cursorEnabled ? DisableCursor() : EnableCursor(); }
		inline GFX::Graphics& Gfx() noexcept { return *graphics; }

		static std::optional<unsigned long long> ProcessMessage() noexcept;

		void SetTitle(const std::string& title);
		void EnableCursor() noexcept;
		void DisableCursor() noexcept;
	};
}
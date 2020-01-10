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
			constexpr static const char * wndClassName = "horusEngineWindow";
			HINSTANCE hInstance;

		public:
			WindowClass() noexcept;
			WindowClass(const WindowClass &) = delete;
			WindowClass & operator=(const WindowClass &) = delete;
			inline ~WindowClass() { UnregisterClass(wndClassName, hInstance); }

			constexpr static const char * GetName() noexcept { return wndClassName; }
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
		Window(unsigned int width, unsigned int height, const char * name);
		Window(const Window&) = delete;
		Window & operator=(const Window&) = delete;
		~Window();

		constexpr Keyboard & Keyboard() noexcept { return keyboard; }
		constexpr Mouse & Mouse() noexcept { return mouse; }
		constexpr bool IsCursorEnabled() const noexcept { return cursorEnabled; }
		inline void SwitchCursor() noexcept { cursorEnabled ? DisableCursor() : EnableCursor(); }
		
		static std::optional<unsigned long long> ProcessMessage() noexcept;

		GFX::Graphics & Gfx();
		void SetTitle(const std::string & title);
		void EnableCursor() noexcept;
		void DisableCursor() noexcept;

#pragma region Exceptions
		class WindowException : public Exception::WinApiException
		{
			using WinApiException::WinApiException;

		public:
			inline WindowException(unsigned int line, const char * file, HRESULT hResult) noexcept
				: BasicException(line, file), WinApiException(line, file, hResult) {}
			WindowException(const WindowException&) = default;
			WindowException & operator=(const WindowException&) = default;
			virtual ~WindowException() = default;

			inline const char * GetType() const noexcept override { return "Window Exception"; }
		};
		class NoGfxException : public virtual Exception::BasicException
		{
		public:
			inline NoGfxException(unsigned int line, const char * file) noexcept : BasicException(line, file) {}
			NoGfxException(const NoGfxException&) = default;
			NoGfxException & operator=(const NoGfxException&) = default;
			virtual ~NoGfxException() = default;

			inline const char * GetType() const noexcept override { return "No Graphics Exception"; }
		};
#pragma endregion
	};
}

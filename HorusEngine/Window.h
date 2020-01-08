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
		HWND hWnd;
		Keyboard keyboard;
		Mouse mouse;
		std::unique_ptr<GFX::Graphics> graphics = nullptr;

		static LRESULT WINAPI HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		static LRESULT WINAPI HandleMsgStub(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

		LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	public:
		Window(unsigned int width, unsigned int height, const char * name);
		Window(const Window&) = delete;
		Window & operator=(const Window&) = delete;
		~Window();

		constexpr Keyboard & Keyboard() noexcept { return keyboard; }
		constexpr Mouse & Mouse() noexcept { return mouse; }

		static std::optional<unsigned long long> ProcessMessage() noexcept;

		GFX::Graphics & Gfx();
		void SetTitle(const std::string & title);

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

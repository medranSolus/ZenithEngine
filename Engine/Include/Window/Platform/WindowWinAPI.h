#pragma once
#include "Platform/WinAPI/WinAPI.h"
#include "Window/MainWindow.h"

namespace ZE::Window::WinAPI
{
	class WindowWinAPI : public BaseWindow
	{
		class WindowClass final
		{
			static constexpr const char* WND_CLASS_NAME = "ZE_WND";

			HINSTANCE hInstance;

		public:
			WindowClass() noexcept;
			WindowClass(WindowClass&&) = delete;
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator=(WindowClass&&) = delete;
			WindowClass& operator=(const WindowClass&) = delete;
			~WindowClass() { UnregisterClass(WND_CLASS_NAME, hInstance); }

			static constexpr const char* GetName() noexcept { return WND_CLASS_NAME; }

			constexpr HINSTANCE GetInstance() const noexcept { return hInstance; }
		};

		static constexpr DWORD WIN_STYLE = WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU;
		static inline WindowClass wndClass;

		HWND hWnd;
		RECT windowRect;
		std::vector<U8> rawBuffer;

		static LRESULT WINAPI HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		static LRESULT WINAPI HandleMsgStub(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

		LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	protected:
		void ShowCursor() const noexcept override { while (::ShowCursor(TRUE) < 0); }
		void HideCursor() const noexcept override { while (::ShowCursor(FALSE) >= 0); }
		void FreeCursor() const noexcept override { ClipCursor(nullptr); }

		void TrapCursor() const noexcept override;
		void EnterFullscreen() noexcept override;
		void LeaveFullscreen() noexcept override;

	public:
		WindowWinAPI(const char* name, U32 width = 0, U32 height = 0);
		~WindowWinAPI();

		constexpr HWND GetHandle() const noexcept { return hWnd; }
		constexpr U32 GetWidth() const noexcept override { return windowRect.right; }
		constexpr U32 GetHeight() const noexcept override { return windowRect.bottom; }

		std::pair<bool, int> ProcessMessage() noexcept override;
		void SetTitle(const std::string& title) override;
		void NewGuiFrame() const noexcept override;
	};
}
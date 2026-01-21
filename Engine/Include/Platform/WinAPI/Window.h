#pragma once
#include "Platform/WinAPI/WinAPI.h"
#include "Window/BaseWindow.h"
#include "Settings.h"

namespace ZE::WinAPI
{
	// Window implementation for Windows
	class Window final : public ZE::Window::BaseWindow
	{
		// Window class register
		class WindowClass final
		{
			HINSTANCE hInstance;

		public:
			WindowClass() noexcept;
			ZE_CLASS_DELETE(WindowClass);
			~WindowClass() { UnregisterClassW(Utils::ToUTF16(Settings::ENGINE_NAME).c_str(), hInstance); }

			constexpr HINSTANCE GetInstance() const noexcept { return hInstance; }
		};

		static constexpr DWORD WIN_STYLE = WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU;
		static inline WindowClass wndClass;

		HWND wndHandle;
		RECT windowRect;
		std::vector<U8> rawBuffer; // TODO: Replace with Table<>
		bool monitorChanged = false;

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
		Window() = default;
		ZE_CLASS_DELETE(Window);
		virtual ~Window();

		static constexpr HINSTANCE GetInstance() noexcept { return wndClass.GetInstance(); }

		constexpr HWND GetHandle() const noexcept { return wndHandle; }
		constexpr U32 GetWidth() const noexcept override { return Utils::SafeCast<U32>(windowRect.right); }
		constexpr U32 GetHeight() const noexcept override { return Utils::SafeCast<U32>(windowRect.bottom); }
		constexpr bool IsMonitorChanged() const noexcept override { return monitorChanged; }
		constexpr void ProcessMonitorChange() noexcept override { monitorChanged = false; }

		void Init(std::string_view name, U32 width = 0, U32 height = 0) override;
		std::pair<bool, int> ProcessMessage() noexcept override;
		void SetTitle(std::string_view title) override;
		void NewImGuiFrame() const noexcept override;
	};
}
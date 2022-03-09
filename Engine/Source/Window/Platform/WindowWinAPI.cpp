#include "Window/Platform/WindowWinAPI.h"
#include "WarningGuardOn.h"
#include "backends/imgui_impl_win32.h"
#include "WarningGuardOff.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ZE::Window::WinAPI
{
	WindowWinAPI::WindowClass::WindowClass() noexcept : hInstance(GetModuleHandle(nullptr))
	{
		WNDCLASSEX wndClassEx = { 0 };
		wndClassEx.cbClsExtra = 0;
		wndClassEx.cbWndExtra = 0;
		wndClassEx.hInstance = hInstance;
		wndClassEx.cbSize = sizeof(wndClassEx);
		wndClassEx.hbrBackground = (HBRUSH)(COLOR_WINDOW);
		wndClassEx.hCursor = nullptr;
		wndClassEx.lpszMenuName = nullptr;
		wndClassEx.style = CS_OWNDC;
		wndClassEx.lpszClassName = WND_CLASS_NAME;
		wndClassEx.lpfnWndProc = HandleMsgSetup;
		wndClassEx.hIcon = static_cast<HICON>(LoadImage(hInstance, MAKEINTRESOURCE(ZE_APPICON), IMAGE_ICON, 128, 128, 0));
		wndClassEx.hIconSm = static_cast<HICON>(LoadImage(hInstance, MAKEINTRESOURCE(ZE_APPICON), IMAGE_ICON, 32, 32, 0));
		RegisterClassEx(&wndClassEx);
	}

	LRESULT WINAPI WindowWinAPI::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		// Reroute WNDPROC to normal function and save pointer to Window object on WinAPI side to retrieve
		if (msg == WM_NCCREATE)
		{
			const CREATESTRUCTW* const create = reinterpret_cast<CREATESTRUCTW*>(lParam);
			// Passed to CreateWindowEX
			WindowWinAPI* const wnd = static_cast<WindowWinAPI*>(create->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wnd));
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WindowWinAPI::HandleMsgStub));
			return wnd->HandleMsg(hWnd, msg, wParam, lParam);
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	LRESULT WINAPI WindowWinAPI::HandleMsgStub(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		// Get saved Window object
		WindowWinAPI* wnd = reinterpret_cast<WindowWinAPI*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		return wnd->HandleMsg(hWnd, msg, wParam, lParam);
	}

	LRESULT WindowWinAPI::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return 0;
		switch (msg)
		{
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
		// The default window procedure will play a system notification sound
		// when pressing the Alt+Enter if this message is not handled.
		case WM_SYSCHAR:
			return 0;
		case WM_ACTIVATE:
		{
			if (!IsCursorEnabled())
			{
				if (wParam & WA_ACTIVE)
					TrapCursor();
				else
					FreeCursor();
			}
			break;
		}
		case WM_KILLFOCUS:
		{
			// No keys left down after focus changed
			keyboard.ClearStates();
			break;
		}
#pragma region Keyboard
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			if (ImGui::GetIO().WantCaptureKeyboard)
				break;
			if (!(lParam & 0x40000000) || keyboard.IsAutorepeat())
			{
				if ((KF_ALTDOWN & HIWORD(lParam)) && wParam == VK_RETURN)
					SwitchFullscreen();
				else
					keyboard.OnKeyDown(static_cast<U8>(wParam));
			}
			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			keyboard.OnKeyUp(static_cast<U8>(wParam));
			break;
		}
		case WM_CHAR:
		{
			if (ImGui::GetIO().WantCaptureKeyboard)
				break;
			keyboard.OnChar(static_cast<char>(wParam));
			break;
		}
#pragma endregion
#pragma region Mouse
		case WM_LBUTTONDOWN:
		{
			SetForegroundWindow(hWnd);
			if (!IsCursorEnabled())
			{
				HideCursor();
				TrapCursor();
			}
			if (ImGui::GetIO().WantCaptureMouse)
				break;
			const POINTS point = MAKEPOINTS(lParam);
			mouse.OnLeftDown(point.x, point.y);
			break;
		}
		case WM_LBUTTONUP:
		{
			if (ImGui::GetIO().WantCaptureMouse)
				break;
			const POINTS point = MAKEPOINTS(lParam);
			mouse.OnLeftUp(point.x, point.y);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			if (ImGui::GetIO().WantCaptureMouse)
				break;
			const POINTS point = MAKEPOINTS(lParam);
			mouse.OnRightDown(point.x, point.y);
			break;
		}
		case WM_RBUTTONUP:
		{
			if (ImGui::GetIO().WantCaptureMouse)
				break;
			const POINTS point = MAKEPOINTS(lParam);
			mouse.OnRightUp(point.x, point.y);
			break;
		}
		case WM_MOUSEWHEEL:
		{
			if (ImGui::GetIO().WantCaptureMouse)
				break;
			mouse.OnWheelRotation(GET_WHEEL_DELTA_WPARAM(wParam));
			break;
		}
		case WM_MOUSEMOVE:
		{
			if (ImGui::GetIO().WantCaptureMouse)
				break;
			const POINTS point = MAKEPOINTS(lParam);
			// Allow window to capture mouse input when left/righ/middle button are pressed when escaping client area
			if (point.x >= 0 && static_cast<U32>(point.x) < GetWidth()
				&& point.y >= 0 && static_cast<U32>(point.y) < GetHeight())
			{
				mouse.OnMouseMove(point.x, point.y);
				if (!mouse.IsInWindow())
				{
					SetCapture(hWnd);
					mouse.OnEnter();
				}
			}
			else
			{
				if (wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON))
					mouse.OnMouseMove(point.x, point.y);
				else
				{
					ReleaseCapture();
					mouse.OnLeave();
				}
			}
			break;
		}
#pragma endregion
#pragma region Raw mouse input
		case WM_INPUT:
		{
			UINT inputSize = 0;
			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT,
				nullptr, &inputSize, sizeof(RAWINPUTHEADER)) == -1)
				break;
			rawBuffer.resize(inputSize);
			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT,
				rawBuffer.data(), &inputSize, sizeof(RAWINPUTHEADER)) != inputSize)
				break;

			const RAWINPUT& input = reinterpret_cast<const RAWINPUT&>(*rawBuffer.data());
			if (input.header.dwType == RIM_TYPEMOUSE &&
				(input.data.mouse.lLastX != 0 || input.data.mouse.lLastY != 0))
				mouse.OnRawDelta(input.data.mouse.lLastX, input.data.mouse.lLastY);
			break;
		}
#pragma endregion
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	void WindowWinAPI::TrapCursor() const noexcept
	{
		RECT rect;
		GetClientRect(wndHandle, &rect);
		MapWindowPoints(wndHandle, nullptr, reinterpret_cast<POINT*>(&rect), 2);
		ClipCursor(&rect);
	}

	void WindowWinAPI::EnterFullscreen() noexcept
	{
		// Set the window style to a borderless window so the client area fills the entire screen.
		SetWindowLong(wndHandle, GWL_STYLE, WS_POPUP);

		// Query the nearest display device for the window.
		// This is required to set the fullscreen dimensions of the window
		// when using a multi-monitor setup.
		HMONITOR monitor = MonitorFromWindow(wndHandle, MONITOR_DEFAULTTONEAREST);
		MONITORINFOEX monitorInfo = {};
		monitorInfo.cbSize = sizeof(MONITORINFOEX);
		GetMonitorInfo(monitor, &monitorInfo);

		// Get the settings for the primary display. These settings are used
		// to determine the correct position and size to position the window.
		DEVMODE devMode = {};
		devMode.dmSize = sizeof(DEVMODE);
		EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode);

		SetWindowPos(wndHandle, HWND_TOP, devMode.dmPosition.x, devMode.dmPosition.y,
			devMode.dmPosition.x + devMode.dmPelsWidth, devMode.dmPosition.y + devMode.dmPelsHeight,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);
		ShowWindow(wndHandle, SW_MAXIMIZE);
	}

	void WindowWinAPI::LeaveFullscreen() noexcept
	{
		// Restore all the window decorators.
		SetWindowLong(wndHandle, GWL_STYLE, WIN_STYLE);

		SetWindowPos(wndHandle, HWND_NOTOPMOST, windowRect.left, windowRect.top,
			GetWidth(), GetHeight(), SWP_FRAMECHANGED | SWP_NOACTIVATE);
		ShowWindow(wndHandle, SW_NORMAL);
	}

	WindowWinAPI::~WindowWinAPI()
	{
		ImGui_ImplWin32_Shutdown();
		DestroyWindow(wndHandle);
	}

	void WindowWinAPI::Init(const char* name, U32 width, U32 height)
	{
		constexpr DWORD WIN_STYLE_EX = 0;

		if (SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) == NULL)
			throw ZE_WIN_EXCEPT_LAST();
		// Initial DPI since no possible way to know window DPI
		const UINT dpi = GetDpiForSystem();

		if (width == 0 || height == 0)
		{
			windowRect.top = 0;
			windowRect.bottom = GetSystemMetricsForDpi(SM_CYMAXIMIZED, dpi);
			windowRect.right = GetSystemMetricsForDpi(SM_CXMAXIMIZED, dpi);
			windowRect.left = -GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi) * 2;
		}
		else
		{
			// Adjust window client area to match specified size
			windowRect.left = 0;
			windowRect.top = 0;
			windowRect.right = width;
			windowRect.bottom = height;
			if (AdjustWindowRectExForDpi(&windowRect, WIN_STYLE, FALSE, WIN_STYLE_EX, dpi) == 0)
				throw ZE_WIN_EXCEPT_LAST();
			windowRect.right -= windowRect.left;
			windowRect.bottom -= windowRect.top;

			// Adjust window position to desktop window
			RECT desktop = { 0 };
			if (GetClientRect(GetDesktopWindow(), &desktop) == 0)
				throw ZE_WIN_EXCEPT_LAST();
			windowRect.left = (desktop.right - windowRect.right) / 2;
			windowRect.top = (desktop.bottom - windowRect.bottom) / 2;
		}

		wndHandle = CreateWindowEx(WIN_STYLE_EX, WindowClass::GetName(), name, WIN_STYLE,
			windowRect.left, windowRect.top, GetWidth(), GetHeight(),
			nullptr, nullptr, wndClass.GetInstance(), this);
		if (wndHandle == nullptr)
			throw ZE_WIN_EXCEPT_LAST();

		if (width == 0 || height == 0)
		{
			ShowWindow(wndHandle, SW_SHOWMAXIMIZED);
			// Get maximized client area
			if (GetClientRect(wndHandle, &windowRect) == 0)
				throw ZE_WIN_EXCEPT_LAST();
		}
		else
			ShowWindow(wndHandle, SW_SHOW);
		ImGui_ImplWin32_Init(wndHandle);

		RAWINPUTDEVICE rid;
		rid.usUsagePage = 1; // Mouse page
		rid.usUsage = 2; // Mouse usage
		rid.dwFlags = 0;
		rid.hwndTarget = nullptr;
		if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
			throw ZE_WIN_EXCEPT_LAST();
	}

	std::pair<bool, int> WindowWinAPI::ProcessMessage() noexcept
	{
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				return { true, static_cast<int>(msg.wParam) };
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return { false, 0 };
	}

	void WindowWinAPI::SetTitle(const std::string& title)
	{
		if (SetWindowText(wndHandle, title.c_str()) == 0)
			throw ZE_WIN_EXCEPT_LAST();
	}

	void WindowWinAPI::NewGuiFrame() const noexcept
	{
		ImGui_ImplWin32_NewFrame();
	}
}
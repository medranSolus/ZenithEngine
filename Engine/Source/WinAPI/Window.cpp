#include "WinAPI/Window.h"
#include "Exception/WinApiException.h"
#include "imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ZE::WinAPI
{
	Window::WindowClass::WindowClass() noexcept : hInstance(GetModuleHandle(nullptr))
	{
		WNDCLASSEX wndClass = { 0 };
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = hInstance;
		wndClass.cbSize = sizeof(wndClass);
		wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW);
		wndClass.hCursor = nullptr;
		wndClass.lpszMenuName = nullptr;
		wndClass.style = CS_OWNDC;
		wndClass.lpszClassName = WND_CLASS_NAME;
		wndClass.lpfnWndProc = HandleMsgSetup;
		wndClass.hIcon = static_cast<HICON>(LoadImage(hInstance, MAKEINTRESOURCE(ZE_APPICON), IMAGE_ICON, 128, 128, 0));
		wndClass.hIconSm = static_cast<HICON>(LoadImage(hInstance, MAKEINTRESOURCE(ZE_APPICON), IMAGE_ICON, 32, 32, 0));
		RegisterClassEx(&wndClass);
	}

	LRESULT WINAPI Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		// Reroute WNDPROC to normal function and save pointer to Window object on WinAPI side to retrieve
		if (msg == WM_NCCREATE)
		{
			const CREATESTRUCTW* const create = reinterpret_cast<CREATESTRUCTW*>(lParam);
			// Passed to CreateWindowEX
			Window* const wnd = static_cast<Window*>(create->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wnd));
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgStub));
			return wnd->HandleMsg(hWnd, msg, wParam, lParam);
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	LRESULT WINAPI Window::HandleMsgStub(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		// Get saved Window object
		Window* wnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		return wnd->HandleMsg(hWnd, msg, wParam, lParam);
	}

	LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;
		switch (msg)
		{
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_SIZE:
		{
			if (wParam != SIZE_MINIMIZED)
			{
				const U32 width = LOWORD(lParam);
				const U32 height = HIWORD(lParam);
				ImGui::GetIO().DisplaySize =
				{
					static_cast<float>(width),
					static_cast<float>(height)
				};
				try
				{
					graphics.Resize(width, height);
				}
				catch (...) {}
			}
			break;
		}
		case WM_ACTIVATE:
		{
			if (!cursorEnabled)
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
				{
					ImGui::GetIO().DisplaySize =
					{
						static_cast<float>(graphics.GetWidth()),
						static_cast<float>(graphics.GetHeight())
					};
					try
					{
						graphics.ToggleFullscreen();
					}
					catch (...) {}
				}
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
			if (!cursorEnabled)
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
			if (point.x >= 0 && static_cast<U32>(point.x) < graphics.GetWidth()
				&& point.y >= 0 && static_cast<U32>(point.y) < graphics.GetHeight())
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
			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &inputSize, sizeof(RAWINPUTHEADER)) == -1)
				break;
			rawBuffer.resize(inputSize);
			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawBuffer.data(), &inputSize, sizeof(RAWINPUTHEADER)) != inputSize)
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

	void Window::TrapCursor() noexcept
	{
		RECT rect;
		GetClientRect(hWnd, &rect);
		MapWindowPoints(hWnd, nullptr, reinterpret_cast<POINT*>(&rect), 2);
		ClipCursor(&rect);
	}

	Window::Window(const char* name, U32 width, U32 height)
	{
		constexpr DWORD WIN_STYLE = WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU;
		constexpr DWORD WIN_STYLE_EX = 0;

		if (SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) == FALSE)
			throw ZE_WIN_EXCEPT_LAST();
		// Initial DPI since no possible way to know window DPI
		dpi = GetDeviceCaps(GetDC(NULL), LOGPIXELSX);

		RECT area = { 0 };
		if (width == 0 || height == 0)
		{
			area.right = GetSystemMetricsForDpi(SM_CXMAXIMIZED, dpi);
			area.bottom = GetSystemMetricsForDpi(SM_CYMAXIMIZED, dpi);
			area.left = -GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi) * 2;
		}
		else
		{
			// Adjust window client area to match specified size
			area.right = width;
			area.bottom = height;
			if (AdjustWindowRectExForDpi(&area, WIN_STYLE, FALSE, WIN_STYLE_EX, dpi) == 0)
				throw ZE_WIN_EXCEPT_LAST();
			area.right -= area.left;
			area.bottom -= area.top;

			// Adjust window position to desktop window
			RECT desktop = { 0 };
			if (GetClientRect(GetDesktopWindow(), &desktop) == 0)
				throw ZE_WIN_EXCEPT_LAST();
			area.left = (desktop.right - area.right) / 2;
			area.top = (desktop.bottom - area.bottom) / 2;
		}

		hWnd = CreateWindowEx(WIN_STYLE_EX, WindowClass::GetName(), name, WIN_STYLE,
			area.left, area.top, area.right, area.bottom,
			nullptr, nullptr, wndClass.GetInstance(), this);
		if (hWnd == nullptr)
			throw ZE_WIN_EXCEPT_LAST();

		if (width == 0 || height == 0)
		{
			ShowWindow(hWnd, SW_SHOWMAXIMIZED);
			// Get maximized client area
			if (GetClientRect(hWnd, &area) == 0)
				throw ZE_WIN_EXCEPT_LAST();
			width = area.right;
			height = area.bottom;
		}
		else
			ShowWindow(hWnd, SW_SHOW);
		// Update DPI to proper window DPI (should be same as previous but not sure)
		dpi = GetDpiForWindow(hWnd);

		graphics.Init(hWnd, width, height);
		ImGui_ImplWin32_Init(hWnd);

		RAWINPUTDEVICE rid;
		rid.usUsagePage = 1; // Mouse page
		rid.usUsage = 2; // Mouse usage
		rid.dwFlags = 0;
		rid.hwndTarget = nullptr;
		if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
			throw ZE_WIN_EXCEPT_LAST();
	}

	Window::~Window()
	{
		ImGui_ImplWin32_Shutdown();
		DestroyWindow(hWnd);
	}

	std::pair<bool, int> Window::ProcessMessage() noexcept
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

	void Window::SetTitle(const std::string& title)
	{
		if (SetWindowText(hWnd, title.c_str()) == 0)
			throw ZE_WIN_EXCEPT_LAST();
	}

	void Window::EnableCursor() noexcept
	{
		if (!cursorEnabled)
		{
			cursorEnabled = true;
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
			ShowCursor();
			FreeCursor();
		}
	}

	void Window::DisableCursor() noexcept
	{
		if (cursorEnabled)
		{
			cursorEnabled = false;
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
			HideCursor();
			TrapCursor();
		}
	}
}
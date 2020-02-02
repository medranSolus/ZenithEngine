#include "WindowExceptionMacros.h"
#include "resource.h"
#include "ImGui/imgui_impl_win32.h"

namespace WinAPI
{
	Window::WindowClass::WindowClass() noexcept : hInstance(GetModuleHandle(nullptr))
	{
		WNDCLASSEX wndClass = { 0 };
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = hInstance;
		wndClass.cbSize = sizeof(wndClass);
		wndClass.hbrBackground = nullptr;
		wndClass.hCursor = nullptr;
		wndClass.hIcon = static_cast<HICON>(LoadImage(GetInstance(), MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 128, 128, 0));
		wndClass.hIconSm = static_cast<HICON>(LoadImage(GetInstance(), MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 32, 32, 0));
		wndClass.lpszMenuName = nullptr;
		wndClass.style = CS_OWNDC;
		wndClass.lpszClassName = wndClassName;
		wndClass.lpfnWndProc = HandleMsgSetup;
		RegisterClassEx(&wndClass);
	}

	Window::WindowClass Window::wndClass;

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
		Window* const wnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
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
				keyboard.OnKeyDown(static_cast<unsigned char>(wParam));
			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			keyboard.OnKeyUp(static_cast<unsigned char>(wParam));
			break;
		}
		case WM_CHAR:
		{
			if (ImGui::GetIO().WantCaptureKeyboard)
				break;
			keyboard.OnChar(static_cast<unsigned char>(wParam));
			break;
		}
#pragma endregion
#pragma region Mouse Input
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
			if (point.x >= 0 && static_cast<unsigned int>(point.x) < wndWidth&& point.y >= 0 && static_cast<unsigned int>(point.y) < wndHeight)
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
#pragma region Raw Input
		case WM_INPUT:
		{
			UINT inputSize = 0U;
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

	Window::Window(unsigned int width, unsigned int height, const char* name) : wndWidth(width), wndHeight(height)
	{
		constexpr DWORD winStyle = WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU;
		constexpr DWORD winStyleEx = 0;
		// Adjust window client area to match specified size
		RECT area = { 0 };
		area.right = wndWidth + area.left;
		area.bottom = wndHeight + area.top;
		if (AdjustWindowRectEx(&area, winStyle, FALSE, winStyleEx) == 0)
			throw WND_EXCEPT_LAST();
		hWnd = CreateWindowEx(winStyleEx, WindowClass::GetName(), name, winStyle,
			CW_USEDEFAULT, CW_USEDEFAULT, area.right - area.left, area.bottom - area.top,
			nullptr, nullptr, wndClass.GetInstance(), this);
		if (hWnd == nullptr)
			throw WND_EXCEPT_LAST();
		ShowWindow(hWnd, SW_SHOW);
		graphics = std::make_unique<GFX::Graphics>(hWnd, width, height);
		ImGui_ImplWin32_Init(hWnd);

		RAWINPUTDEVICE rid;
		rid.usUsagePage = 1; // Mouse page
		rid.usUsage = 2; // Mouse usage
		rid.dwFlags = 0;
		rid.hwndTarget = nullptr;
		if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
			throw WND_EXCEPT_LAST();
	}

	Window::~Window()
	{
		ImGui_ImplWin32_Shutdown();
		DestroyWindow(hWnd);
	}

	std::optional<unsigned long long> Window::ProcessMessage() noexcept
	{
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				return static_cast<unsigned long long>(msg.wParam);
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return {};
	}

	GFX::Graphics& Window::Gfx()
	{
		if (!graphics)
			throw WND_NO_GFX_EXCEPT();
		return *graphics;
	}

	void Window::SetTitle(const std::string& title)
	{
		if (SetWindowText(hWnd, title.c_str()) == 0)
			throw WND_EXCEPT_LAST();
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
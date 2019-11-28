#include "Mouse.h"
#include "WinAPI.h"

namespace WinAPI
{
	void Mouse::OnLeftDown(int x, int y) noexcept
	{
		this->x = x;
		this->y = y;
		left = true;
		eventBuffer.emplace_back(Event::Type::LeftDown, *this);
		TrimBuffer();
	}

	void Mouse::OnLeftUp(int x, int y) noexcept
	{
		this->x = x;
		this->y = y;
		left = false;
		eventBuffer.emplace_back(Event::Type::LeftUp, *this);
		TrimBuffer();
	}

	void Mouse::OnRightDown(int x, int y) noexcept
	{
		this->x = x;
		this->y = y;
		right = true;
		eventBuffer.emplace_back(Event::Type::RightDown, *this);
		TrimBuffer();
	}

	void Mouse::OnRightUp(int x, int y) noexcept
	{
		this->x = x;
		this->y = y;
		right = false;
		eventBuffer.emplace_back(Event::Type::RightUp, *this);
		TrimBuffer();
	}

	void Mouse::OnWheelDown(int x, int y) noexcept
	{
		this->x = x;
		this->y = y;
		wheel = true;
		eventBuffer.emplace_back(Event::Type::WheelDown, *this);
		TrimBuffer();
	}

	void Mouse::OnWheelUp(int x, int y) noexcept
	{
		this->x = x;
		this->y = y;
		wheel = false;
		eventBuffer.emplace_back(Event::Type::WheelUp, *this);
		TrimBuffer();
	}

	void Mouse::OnWheelForward(int x, int y) noexcept
	{
		this->x = x;
		this->y = y;
		eventBuffer.emplace_back(Event::Type::WheelForward, *this);
		TrimBuffer();
	}

	void Mouse::OnWheelBackward(int x, int y) noexcept
	{
		this->x = x;
		this->y = y;
		eventBuffer.emplace_back(Event::Type::WheelBackward, *this);
		TrimBuffer();
	}

	void Mouse::OnWheelRotation(int x, int y, int rotation) noexcept
	{
		// Rotation for high precision mouse wheels https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mousewheel
		wheelRotation += rotation;
		while (wheelRotation >= WHEEL_DELTA)
		{
			wheelRotation -= WHEEL_DELTA;
			OnWheelForward(x, y);
		}
		while (wheelRotation <= -WHEEL_DELTA)
		{
			wheelRotation += WHEEL_DELTA;
			OnWheelBackward(x, y);
		}
	}

	void Mouse::OnMouseMove(int x, int y) noexcept
	{
		this->x = x;
		this->y = y;
		eventBuffer.emplace_back(Event::Type::Move, *this);
		TrimBuffer();
	}

	void Mouse::OnEnter() noexcept
	{
		window = true;
		eventBuffer.emplace_back(Event::Type::Enter, *this);
		TrimBuffer();
	}

	void Mouse::OnLeave() noexcept
	{
		window = false;
		eventBuffer.emplace_back(Event::Type::Leave, *this);
		TrimBuffer();
	}

	void Mouse::TrimBuffer() noexcept
	{
		while (eventBuffer.size() > bufferSize)
			eventBuffer.pop_back();
	}

	std::optional<Mouse::Event> Mouse::Read() noexcept
	{
		if (eventBuffer.size() > 0)
		{
			Event input = eventBuffer.front();
			eventBuffer.pop_front();
			return input;
		}
		return {};
	}
}

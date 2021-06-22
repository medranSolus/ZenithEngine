#include "Window/Mouse.h"

namespace ZE::Window
{
	void Mouse::TrimBuffer() noexcept
	{
		while (eventBuffer.size() > BUFFER_SIZE)
			eventBuffer.pop_back();
	}

	void Mouse::OnLeftDown(S32 x, S32 y) noexcept
	{
		left = true;
		eventBuffer.emplace_back(Event::Type::LeftDown, *this, x, y);
		this->x = x;
		this->y = y;
		TrimBuffer();
	}

	void Mouse::OnLeftUp(S32 x, S32 y) noexcept
	{
		left = false;
		eventBuffer.emplace_back(Event::Type::LeftUp, *this, x, y);
		this->x = x;
		this->y = y;
		TrimBuffer();
	}

	void Mouse::OnRightDown(S32 x, S32 y) noexcept
	{
		right = true;
		eventBuffer.emplace_back(Event::Type::RightDown, *this, x, y);
		this->x = x;
		this->y = y;
		TrimBuffer();
	}

	void Mouse::OnRightUp(S32 x, S32 y) noexcept
	{
		right = false;
		eventBuffer.emplace_back(Event::Type::RightUp, *this, x, y);
		this->x = x;
		this->y = y;
		TrimBuffer();
	}

	void Mouse::OnWheelDown(S32 x, S32 y) noexcept
	{
		wheel = true;
		eventBuffer.emplace_back(Event::Type::WheelDown, *this, x, y);
		this->x = x;
		this->y = y;
		TrimBuffer();
	}

	void Mouse::OnWheelUp(S32 x, S32 y) noexcept
	{
		wheel = false;
		eventBuffer.emplace_back(Event::Type::WheelUp, *this, x, y);
		this->x = x;
		this->y = y;
		TrimBuffer();
	}

	void Mouse::OnMouseMove(S32 x, S32 y) noexcept
	{
		eventBuffer.emplace_back(Event::Type::Move, *this, x, y);
		this->x = x;
		this->y = y;
		TrimBuffer();
	}

	void Mouse::OnRawDelta(S32 dx, S32 dy) noexcept
	{
		this->x += dx;
		this->y += dy;
		eventBuffer.emplace_back(Event::Type::RawMove, *this, Event::RawInput({ dx, dy }));
		TrimBuffer();
	}

	void Mouse::OnWheelRotation(S32 rotation) noexcept
	{
		wheelRotation += rotation;
		while (wheelRotation >= WHEEL_TRESHOLD)
		{
			wheelRotation -= WHEEL_TRESHOLD;
			OnWheelForward();
		}
		while (wheelRotation <= -WHEEL_TRESHOLD)
		{
			wheelRotation += WHEEL_TRESHOLD;
			OnWheelBackward();
		}
	}

	void Mouse::OnWheelForward() noexcept
	{
		eventBuffer.emplace_back(Event::Type::WheelForward, *this, x, y);
		TrimBuffer();
	}

	void Mouse::OnWheelBackward() noexcept
	{
		eventBuffer.emplace_back(Event::Type::WheelBackward, *this, x, y);
		TrimBuffer();
	}

	void Mouse::OnEnter() noexcept
	{
		window = true;
		eventBuffer.emplace_back(Event::Type::Enter, *this, x, y);
		TrimBuffer();
	}

	void Mouse::OnLeave() noexcept
	{
		window = false;
		eventBuffer.emplace_back(Event::Type::Leave, *this, x, y);
		TrimBuffer();
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
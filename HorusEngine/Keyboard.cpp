#include "Keyboard.h"

namespace WinAPI
{
	template<typename T>
	void Keyboard::TrimBuffer(std::deque<T> & buffer) noexcept
	{
		while (buffer.size() > bufferSize)
			buffer.pop_back();
	}

	void Keyboard::OnKeyDown(unsigned char keycode) noexcept
	{
		keystates[keycode] = true;
		keybuffer.emplace_back(Keyboard::Event::Type::Down, keycode);
		TrimBuffer(keybuffer);
	}

	void Keyboard::OnKeyUp(unsigned char keycode) noexcept
	{
		keystates[keycode] = false;
		keybuffer.emplace_back(Keyboard::Event::Type::Up, keycode);
		TrimBuffer(keybuffer);
	}

	void Keyboard::OnChar(char character) noexcept
	{
		charbuffer.emplace_back(character);
		TrimBuffer(charbuffer);
	}

	void Keyboard::Flush() noexcept
	{
		FlushKeys();
		FlushChars();
	}

	std::optional<Keyboard::Event> Keyboard::ReadKey() noexcept
	{
		if (keybuffer.size() > 0)
		{
			Keyboard::Event e = keybuffer.front();
			keybuffer.pop_front();
			return e;
		}
		return {};
	}

	std::optional<char> Keyboard::ReadChar() noexcept
	{
		if (charbuffer.size() > 0)
		{
			char charcode = charbuffer.front();
			charbuffer.pop_front();
			return charcode;
		}
		return {};
	}
}

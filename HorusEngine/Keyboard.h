#pragma once
#include <bitset>
#include <deque>
#include <optional>

namespace WinAPI
{
	class Keyboard
	{
		friend class Window;
	public:
		class Event
		{
		public:
			enum Type : bool { Down, Up };

		private:
			Type type;
			unsigned char code;

		public:
			constexpr Event(Type type, unsigned char code) noexcept : type(type), code(code) {}
			constexpr Event(const Event &) = default;
			constexpr Event & operator=(const Event &) = default;
			~Event() = default;

			constexpr bool IsDown() const noexcept { return type == Type::Down; }
			constexpr bool IsUp() const noexcept { return type == Type::Up; }
			constexpr unsigned char GetCode() const noexcept { return code; }
		};

	private:
		constexpr static unsigned int numberOfVKeys = 256U;
		constexpr static unsigned int bufferSize = 32U;

		bool autorepeatEnabled = false; // Accounting long key press
		std::bitset<numberOfVKeys> keystates; // States for all virtual keys from WinAPI
		std::deque<Event> keybuffer; // Buffer for KEY_UP/DOWN events
		std::deque<char> charbuffer; // Buffer for ON_CHAR events

		inline void ClearStates() noexcept { keystates.reset(); }

		void OnKeyDown(unsigned char keycode) noexcept;
		void OnKeyUp(unsigned char keycode) noexcept;
		void OnChar(char character) noexcept;
		template<typename T>
		static void TrimBuffer(std::deque<T>& buffer) noexcept;

	public:
		Keyboard() = default;
		constexpr Keyboard(const Keyboard&) = delete;
		constexpr Keyboard & operator=(const Keyboard &) = delete;
		~Keyboard() = default;

		constexpr void EnableAutorepeat() noexcept { autorepeatEnabled = false; }
		constexpr void DisableAutorepeat() noexcept { autorepeatEnabled = true; }
		constexpr bool IsAutorepeat() const noexcept { return autorepeatEnabled; }

		constexpr bool IsKeyDown(unsigned char keycode) const noexcept { return keystates[keycode]; }
		inline bool IsKeyReady() const noexcept { return keybuffer.size(); }
		inline bool IsCharReady() const noexcept { return charbuffer.size(); }
		inline void FlushKeys() noexcept { keybuffer = std::deque<Event>(); }
		inline void FlushChars() noexcept { charbuffer = std::deque<char>(); }

		void Flush() noexcept;
		std::optional<Event> ReadKey() noexcept;
		std::optional<char> ReadChar() noexcept;
	};
}

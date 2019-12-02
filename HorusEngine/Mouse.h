#pragma once
#include <deque>
#include <optional>

namespace WinAPI
{
	class Mouse
	{
		friend class Window;
	public:
		class Event
		{
		public:
			enum Type : char { LeftUp, LeftDown, RightUp, RightDown, WheelUp, WheelDown, WheelForward, WheelBackward, Move, Enter, Leave };

		private:
			Type type;
			int x;
			int y;
			int dX;
			int dY;
			bool left;
			bool right;
			bool wheel;

		public:
			Event(Type type, const Mouse & mouse, int x, int y) noexcept
				: type(type), x(x), y(y), dX(x - mouse.x), dY(y - mouse.y), left(mouse.left), right(mouse.right), wheel(mouse.wheel) {}
			constexpr Event(const Event &) = default;
			constexpr Event & operator=(const Event &) = default;
			~Event() = default;

			constexpr Type GetType() const noexcept { return type; }
			constexpr std::pair<int, int> GetPosition() const noexcept { return std::move(std::make_pair(x, y)); }
			constexpr int GetX() const noexcept { return x; }
			constexpr int GetY() const noexcept { return y; }
			constexpr int GetDX() const noexcept { return dX; }
			constexpr int GetDY() const noexcept { return dY; }
			constexpr bool IsLeftDown() const noexcept { return left; }
			constexpr bool IsRightDown() const noexcept { return right; }
			constexpr bool IsWheelDown() const noexcept { return wheel; }
		};

	private:
		constexpr static unsigned int bufferSize = 64U;

		int x = 0;
		int y = 0;
		int wheelRotation = 0;
		bool left = false;
		bool right = false;
		bool wheel = false;
		bool window = false;
		std::deque<Event> eventBuffer;

		void OnLeftDown(int x, int y) noexcept;
		void OnLeftUp(int x, int y) noexcept;
		void OnRightDown(int x, int y) noexcept;
		void OnRightUp(int x, int y) noexcept;
		void OnWheelDown(int x, int y) noexcept;
		void OnWheelUp(int x, int y) noexcept;
		void OnWheelForward(int x, int y) noexcept;
		void OnWheelBackward(int x, int y) noexcept;
		void OnWheelRotation(int x, int y, int rotation) noexcept;
		void OnMouseMove(int x, int y) noexcept;
		void OnEnter() noexcept;
		void OnLeave() noexcept;
		void TrimBuffer() noexcept;

	public:
		Mouse() = default;
		constexpr Mouse(const Mouse &) = delete;
		constexpr Mouse & operator=(const Mouse &) = delete;
		~Mouse() = default;

		constexpr std::pair<int, int> GetPosition() const noexcept { return std::move(std::make_pair(x, y)); }
		constexpr inline int GetX() const noexcept { return x; }
		constexpr inline int GetY() const noexcept { return y; }
		constexpr inline bool IsLeftDown() const noexcept { return left; }
		constexpr inline bool IsRightDown() const noexcept { return right; }
		constexpr inline bool IsWheelDown() const noexcept { return wheel; }
		constexpr inline bool IsInWindow() const noexcept { return window; }
		inline bool IsInput() const noexcept { return eventBuffer.size(); }
		inline void Flush() noexcept { eventBuffer = std::deque<Event>(); }

		std::optional<Event> Read() noexcept;
	};
}

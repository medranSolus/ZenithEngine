#pragma once
#include "Types.h"
#include <optional>

namespace ZE::Window
{
#ifdef _ZE_PLATFORM_WINDOWS
	namespace WinAPI
	{
		class WindowWinAPI;
	}
#else
#error Missing platform specyfic window forward declaration for Mouse!
#endif

	// Keyboard events controller
	class Mouse final
	{
		friend class WinAPI::WindowWinAPI;

	public:
		class Event final
		{
		public:
			enum class Type : U8
			{
				LeftUp, LeftDown, RightUp, RightDown,
				WheelUp, WheelDown, WheelForward, WheelBackward,
				Move, RawMove, Enter, Leave
			};
			struct RawInput
			{
				S32 dX;
				S32 dY;
			};

		private:
			Type type;
			S32 x;
			S32 y;
			RawInput delta;
			bool left;
			bool right;
			bool wheel;

		public:
			constexpr Event(Type type, const Mouse& mouse, S32 x, S32 y) noexcept
				: type(type), x(x), y(y), delta({ x - mouse.currX, y - mouse.currY }), left(mouse.left), right(mouse.right), wheel(mouse.wheel) {}
			constexpr Event(Type type, const Mouse& mouse, RawInput delta) noexcept
				: type(type), x(mouse.currX), y(mouse.currY), delta(delta), left(mouse.left), right(mouse.right), wheel(mouse.wheel) {}
			ZE_CLASS_DEFAULT(Event);
			~Event() = default;

			constexpr Type GetType() const noexcept { return type; }
			constexpr std::pair<S32, S32> GetPosition() const noexcept { return { x, y }; }
			constexpr S32 GetX() const noexcept { return x; }
			constexpr S32 GetY() const noexcept { return y; }
			constexpr S32 GetDX() const noexcept { return delta.dX; }
			constexpr S32 GetDY() const noexcept { return delta.dY; }
			constexpr bool IsLeftDown() const noexcept { return left; }
			constexpr bool IsRightDown() const noexcept { return right; }
			constexpr bool IsWheelDown() const noexcept { return wheel; }
		};

	private:
		static constexpr size_t BUFFER_SIZE = 256;
		// Rotation for high precision mouse wheels https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mousewheel
		static constexpr S32 WHEEL_TRESHOLD = 120;

		S32 currX = 0;
		S32 currY = 0;
		S32 wheelRotation = 0;
		bool left = false;
		bool right = false;
		bool wheel = false;
		bool window = false;
		std::deque<Event> eventBuffer;

		void TrimBuffer() noexcept;
		void OnLeftDown(S32 x, S32 y) noexcept;
		void OnLeftUp(S32 x, S32 y) noexcept;
		void OnRightDown(S32 x, S32 y) noexcept;
		void OnRightUp(S32 x, S32 y) noexcept;
		void OnWheelDown(S32 x, S32 y) noexcept;
		void OnWheelUp(S32 x, S32 y) noexcept;
		void OnMouseMove(S32 x, S32 y) noexcept;
		void OnRawDelta(S32 dx, S32 dy) noexcept;
		void OnWheelRotation(S32 rotation) noexcept;
		void OnWheelForward() noexcept;
		void OnWheelBackward() noexcept;
		void OnEnter() noexcept;
		void OnLeave() noexcept;

	public:
		Mouse() = default;
		ZE_CLASS_DELETE(Mouse);
		~Mouse() = default;

		constexpr std::pair<S32, S32> GetPosition() const noexcept { return { GetX(), GetY() }; }
		constexpr S32 GetX() const noexcept { return currX; }
		constexpr S32 GetY() const noexcept { return currY; }
		constexpr bool IsLeftDown() const noexcept { return left; }
		constexpr bool IsRightDown() const noexcept { return right; }
		constexpr bool IsWheelDown() const noexcept { return wheel; }
		constexpr bool IsInWindow() const noexcept { return window; }

		bool IsInput() const noexcept { return static_cast<bool>(eventBuffer.size()); }
		void Flush() noexcept { eventBuffer = std::deque<Event>{}; }

		std::optional<Event> Read() noexcept;
	};
}
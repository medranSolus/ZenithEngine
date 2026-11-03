#pragma once

namespace ZE
{
	// Wrapper for plain types to allow move semantics to clear the value to a default one after moving
	template<typename T, T DEF_VALUE>
	class Move final
	{
		T value = DEF_VALUE;

	public:
		Move() = default;
		~Move() = default;

		// Initialization from T
		constexpr Move(T&& m) noexcept : value(std::move(m)) {}
		constexpr Move(const T& m) noexcept : value(m) {}
		constexpr Move& operator=(T&& m) noexcept { value = std::move(m); return *this; }
		constexpr Move& operator=(const T& m) noexcept { value = m; return *this; }

		// Initialization from compatible Move<T, DEF_VALUE>
		template<T DEF_VAL>
		constexpr Move(Move<T, DEF_VAL>&& m) noexcept : value(std::exchange(m.value, DEF_VAL)) {}
		template<T DEF_VAL>
		constexpr Move(const Move<T, DEF_VAL>& m) noexcept : value(m.value) {}
		template<T DEF_VAL>
		constexpr Move& operator=(Move<T, DEF_VAL>&& m) noexcept { value = std::exchange(m.value, DEF_VAL); return *this; }
		template<T DEF_VAL>
		constexpr Move& operator=(const Move<T, DEF_VAL>& m) noexcept { value = m.value; return *this; }

		operator const T&() const noexcept { return value; }
		operator T&() noexcept { return value; }
	};
}
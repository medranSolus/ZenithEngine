#pragma once
#include "Settings.h"

namespace ZE::GFX
{
	// Chain of resources for every frame in flight
	template<typename T>
	class ChainPool final
	{
		// TODO: Seems to be slower for now, investigate later
		union Inner
		{
			Ptr<T> ptr;
			T impl;

			constexpr Inner() noexcept {}
			ZE_CLASS_NO_COPY(Inner);
			constexpr Inner(Inner&& i) noexcept;
			constexpr Inner& operator=(Inner&& i) noexcept;
			~Inner() {}
		} inner;

	public:
		constexpr ChainPool() noexcept;
		ZE_CLASS_MOVE(ChainPool);
		~ChainPool();

		// Get current resource
		constexpr T& Get() noexcept;
		// Get current resource
		constexpr const T& Get() const noexcept;
		// Execute function on every inner resource, ex. when resources need special init/destroy or to alter their state
		constexpr void Exec(std::function<void(T&)> x) noexcept;
		// Execute function on every inner resource, and stop execution if any of them returned error status
		constexpr Status ExecStatus(std::function<Status(T&)> x) noexcept;
	};

#pragma region Functions
	template<typename T>
	constexpr ChainPool<T>::Inner::Inner(Inner&& i) noexcept
	{
		if (Settings::GetChainResourceCount() > 1)
			ptr = std::exchange(i.ptr, nullptr);
		else
			impl = std::move(i.impl);
	}

	template<typename T>
	constexpr ChainPool<T>::Inner& ChainPool<T>::Inner::operator=(Inner&& i) noexcept
	{
		if (Settings::GetChainResourceCount() > 1)
			ptr = std::exchange(i.ptr, nullptr);
		else
			impl = std::move(i.impl);
		return *this;
	}

	template<typename T>
	constexpr ChainPool<T>::ChainPool() noexcept
	{
		if (Settings::GetChainResourceCount() > 1)
			inner.ptr = new T[Settings::GetChainResourceCount()];
		else
			new(&inner.impl) T;
	}

	template<typename T>
	ChainPool<T>::~ChainPool()
	{
		if (Settings::GetChainResourceCount() > 1)
		{
			if (inner.ptr)
				inner.ptr.DeleteArray();
		}
		else
			inner.impl.~T();
	}

	template<typename T>
	constexpr T& ChainPool<T>::Get() noexcept
	{
		if (Settings::GetChainResourceCount() > 1)
			return inner.ptr[Settings::GetCurrentChainResourceIndex()];
		return inner.impl;
	}

	template<typename T>
	constexpr const T& ChainPool<T>::Get() const noexcept
	{
		if (Settings::GetChainResourceCount() > 1)
			return inner.ptr[Settings::GetCurrentChainResourceIndex()];
		return inner.impl;
	}

	template<typename T>
	constexpr void ChainPool<T>::Exec(std::function<void(T&)> x) noexcept
	{
		if (Settings::GetChainResourceCount() > 1)
		{
			for (U32 i = Settings::GetChainResourceCount(); i;)
				x(inner.ptr[--i]);
		}
		else
			x(inner.impl);
	}

	template<typename T>
	constexpr Status ChainPool<T>::ExecStatus(std::function<Status(T&)> x) noexcept
	{
		if (Settings::GetChainResourceCount() > 1)
		{
			for (U32 i = Settings::GetChainResourceCount(); i;)
			{
				ZE_CODE_RET_FAILED(x(inner.ptr[--i]));
			}
			return {};
		}
		else
		{
			ZE_CODE_RET_FAILED(x(inner.impl));
		}
	}
#pragma endregion
}
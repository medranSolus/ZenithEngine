#pragma once
#include "Settings.h"

namespace ZE::GFX
{
	// Chain of resources for every frame in flight
	template<typename T>
	class ChainPool final
	{
		// TODO: Seems to be slower for now, investigate later
		union
		{
			Ptr<T> ptr;
			T impl;
		};

	public:
		constexpr ChainPool();
		ZE_CLASS_MOVE(ChainPool);
		~ChainPool();

		// Get current resource
		constexpr T& Get() noexcept;
		// Get current resource
		constexpr const T& Get() const noexcept;
		// Execute function on every inner resource, ex. when resources need special init/destroy or to alter their state
		constexpr void Exec(std::function<void(T&)> x);
	};

#pragma region Functions
	template<typename T>
	constexpr ChainPool<T>::ChainPool()
	{
		if (Settings::GetChainResourceCount() > 1)
			ptr = new T[Settings::GetChainResourceCount()];
		else
			new(&impl) T;
	}

	template<typename T>
	ChainPool<T>::~ChainPool()
	{
		if (Settings::GetChainResourceCount() > 1)
		{
			if (ptr)
				ptr.DeleteArray();
		}
		else
			impl.~T();
	}

	template<typename T>
	constexpr T& ChainPool<T>::Get() noexcept
	{
		if (Settings::GetChainResourceCount() > 1)
			return ptr[Settings::GetCurrentChainResourceIndex()];
		return impl;
	}

	template<typename T>
	constexpr const T& ChainPool<T>::Get() const noexcept
	{
		if (Settings::GetChainResourceCount() > 1)
			return ptr[Settings::GetCurrentChainResourceIndex()];
		return impl;
	}

	template<typename T>
	constexpr void ChainPool<T>::Exec(std::function<void(T&)> x)
	{
		if (Settings::GetChainResourceCount() > 1)
		{
			for (U32 i = Settings::GetChainResourceCount(); i;)
				x(ptr[--i]);
		}
		else
			x(impl);
	}
#pragma endregion
}
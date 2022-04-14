#pragma once
#include "Settings.h"

namespace ZE::GFX
{
	// Chain of resources for every frame in flight
	template<typename T>
	class ChainPool final
	{
		// TODO: Seems to be slower for now, investigate later
		Ptr<T> pool;

	public:
		constexpr ChainPool() noexcept : pool(new T[Settings::GetBackbufferCount()]) {}
		ZE_CLASS_MOVE(ChainPool);
		~ChainPool() { if (pool) pool.DeleteArray(); }

		// Get current resource
		constexpr T& Get() noexcept { return pool[Settings::GetCurrentChainResourceIndex()]; }
		// Get current resource
		constexpr const T& Get() const noexcept { return pool[Settings::GetCurrentChainResourceIndex()]; }
		// Execute function on every inner resource, ex. when resources need special init/destroy or to alter their state
		constexpr void Exec(std::function<void(T&)> x) { for (U32 i = Settings::GetBackbufferCount(); i;) x(pool[--i]); }
	};
}
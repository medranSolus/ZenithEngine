#pragma once
#include "Settings.h"

namespace ZE::GFX
{
	// Chain of resources for every frame in flight
	template<typename T>
	class ChainPool final
	{
		std::unique_ptr<T[]> pool;

	public:
		constexpr ChainPool() noexcept :pool(std::make_unique<T[]>(Settings::GetChainResourceCount())) {}
		ZE_CLASS_MOVE(ChainPool);
		~ChainPool() = default;

		// Get current resource
		constexpr T& Get() noexcept { return pool[Settings::GetCurrentChainResourceIndex()]; }
		// Get current resource
		constexpr const T& Get() const noexcept { return pool[Settings::GetCurrentChainResourceIndex()]; }
		// Execute function on every inner resource, ex. when resources need special init/destroy or to alter their state
		constexpr void Exec(std::function<void(T&)> x) noexcept;
		// Execute function on every inner resource, and stop execution if any of them returned error status
		constexpr Status ExecStatus(std::function<Status(T&)> x) noexcept;
	};

#pragma region Functions
	template<typename T>
	constexpr void ChainPool<T>::Exec(std::function<void(T&)> x) noexcept
	{
		for (U32 i = Settings::GetChainResourceCount(); i;)
			x(pool[--i]);
	}

	template<typename T>
	constexpr Status ChainPool<T>::ExecStatus(std::function<Status(T&)> x) noexcept
	{
		for (U32 i = Settings::GetChainResourceCount(); i;)
		{
			ZE_CODE_RET_FAILED(x(pool[--i]));
		}
		return {};
	}
#pragma endregion
}
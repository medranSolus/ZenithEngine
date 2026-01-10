#pragma once
#include "BasicTypes.h"
ZE_WARNING_PUSH
ZE_WARNING_DISABLE_MSVC(5204) // Bug in MSVC producing warnings in <future> header
#include <future>
ZE_WARNING_POP
#include <memory>

namespace ZE
{
	// Information about asynchronous task scheduled to separate thread
	template <typename R>
	class Task final
	{
		friend class ThreadPool;

		struct ExecutionData
		{
			std::packaged_task<R()> task;
			BoolAtom processing = false;
		};
		std::shared_ptr<ExecutionData> data;

		constexpr std::shared_ptr<ExecutionData> GetData() noexcept { return data; }

	public:
		Task() = default;
		constexpr Task(std::packaged_task<R()>&& task) noexcept
			: data(std::make_shared<ExecutionData>(std::move(task))) {}
		ZE_CLASS_DEFAULT(Task);
		~Task() = default;

		// Waits for scheduled task completion before returting data if any
		constexpr R Get() noexcept;
	};

#pragma region Functions
	template <typename R>
	constexpr R Task<R>::Get() noexcept
	{
		if (data)
		{
			std::future<R> future = data->task.get_future();
			const bool status = std::atomic_exchange_explicit(&data->processing, true, std::memory_order::memory_order_acq_rel);
			// Check if some thread already started working on this task, if not do it yourself
			if (!status)
				data->task();
			return future.get();
		}
		return R();
	}

#pragma endregion
}
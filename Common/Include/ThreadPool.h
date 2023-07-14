#pragma once
#include "Allocator/BlockingQueue.h"
#include "Allocator/FixedPool.h"
#include "Task.h"
#include <array>
#include <condition_variable>
#include <functional>
#include <thread>

namespace ZE
{
	// Known type of underlying CPU
	enum class VendorCPU : U8 { Intel, IntelHybrid, AMD, AMDOld, Unknown };

	// Available priorites for jobs scheduled into thread pool
	enum class ThreadPriority : U8 { Critical = 0, High = 1, Normal = 2 };

	// Pool managing asynchronous job submissions and delegating them into workers on different threads
	class ThreadPool final
	{
		static inline VendorCPU currentCPU = VendorCPU::Unknown;
		static inline U8 coresCount = 0;
		static inline U8 logicalCoresCount = 0;

		U8 threadsCountOverride = 0;
		U8 maxThreadsCount = UINT8_MAX;
		bool useMultiThreading = true;

		BoolAtom runControl = true;
		Ptr<BoolAtom> threadRunControls;
		std::vector<std::thread> threads;

		mutable std::condition_variable signaler;
		mutable std::array<Allocator::BlockingQueue<std::function<void()>>, 3> taskQueues;

		constexpr void ResizeThreads(U8 oldCount, U8 currentCount) noexcept;
		void Worker(const BoolAtom& run) const noexcept;

	public:
		ThreadPool(U8 customThreadCount = 0) noexcept;
		ZE_CLASS_MOVE(ThreadPool);
		~ThreadPool();

		static constexpr VendorCPU GetCurrentCPU() noexcept { return currentCPU; }
		static constexpr U8 GetCoresCount() noexcept { return coresCount; }
		static constexpr U8 GetLogicalCoresCount() noexcept { return logicalCoresCount; }

		constexpr void ResetThreadsCount() noexcept { ClampThreadsCount(UINT8_MAX); }
		void Stop() noexcept { runControl = false; }

		template <typename Func, typename... Args>
		constexpr auto Schedule(ThreadPriority priority, Func&& f, Args&&... args) const noexcept -> Task<decltype(f(args...))>;

		constexpr U8 GetThreadsCount() const noexcept;
		constexpr void ClampThreadsCount(U8 maxThreads) noexcept;
		constexpr void SetSMT(bool val) noexcept;
		constexpr void Init(U8 customThreadCount = 0) noexcept;
	};

#pragma region Functions
	template <typename Func, typename... Args>
	constexpr auto ThreadPool::Schedule(ThreadPriority priority, Func&& f, Args&&... args) const noexcept -> Task<decltype(f(args...))>
	{
		using Return = decltype(f(args...));

		std::packaged_task<Return()> taskPackage(std::bind(std::forward<Func>(f), std::forward<Args>(args)...));
		Task<Return> task(std::move(taskPackage));
		auto workerFunc = [execData = task.GetData()]() -> void
		{
			const bool status = std::atomic_exchange_explicit(&execData->processing, true, std::memory_order::memory_order_acq_rel);
			if (!status)
				execData->task();
		};
		// When pool is stopped don't delegate new tasks to it, only run them in single thread
		if (GetThreadsCount() > 0 && runControl)
		{
			taskQueues[static_cast<U8>(priority)].EmplaceBack(std::move(workerFunc));
			signaler.notify_one();
		}
		else
			workerFunc();
		return task;
	}

	constexpr U8 ThreadPool::GetThreadsCount() const noexcept
	{
		if (threadsCountOverride != 0)
			return threadsCountOverride == UINT8_MAX ? 0 : threadsCountOverride;
		U8 count;
		switch (currentCPU)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case ZE::VendorCPU::Intel:
		case ZE::VendorCPU::Unknown:
			count = useMultiThreading ? logicalCoresCount : coresCount;
			break;
		case ZE::VendorCPU::AMD:
			count = useMultiThreading || coresCount < 8 ? logicalCoresCount : coresCount;
			break;
		case ZE::VendorCPU::IntelHybrid:
		case ZE::VendorCPU::AMDOld:
			count = logicalCoresCount;
			break;
		}
		return Math::Clamp(static_cast<U8>(count - 1), static_cast<U8>(0), maxThreadsCount);
	}

	constexpr void ThreadPool::ClampThreadsCount(U8 maxThreads) noexcept
	{
		const U8 oldCount = GetThreadsCount();
		maxThreadsCount = maxThreads;
		ResizeThreads(oldCount, GetThreadsCount());
	}

	constexpr void ThreadPool::SetSMT(bool val) noexcept
	{
		const U8 oldCount = GetThreadsCount();
		useMultiThreading = val;
		ResizeThreads(oldCount, GetThreadsCount());
	}

	constexpr void ThreadPool::Init(U8 customThreadCount) noexcept
	{
		if (customThreadCount != UINT8_MAX)
		{
			threadsCountOverride = customThreadCount;

			// Create worker threads that will sleep waiting for new job to execute
			const U8 count = GetThreadsCount();
			threadRunControls = new BoolAtom[count];
			threads.reserve(count);
			for (U8 i = 0; i < count; ++i)
			{
				threadRunControls[i] = true;
				threads.emplace_back(&ThreadPool::Worker, this, std::cref(threadRunControls[i]));
			}
		}
	}
#pragma endregion
}
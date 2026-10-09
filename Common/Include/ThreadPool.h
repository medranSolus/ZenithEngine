#pragma once
#include "Allocator/BlockingQueue.h"
#include "Allocator/FixedPool.h"
#include "Task.h"
#include <array>
#include <condition_variable>
#include <functional>
#include <thread>
#include <vector>

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
		U8 allocatedThreads = 0;
		U8 maxThreadsCount = UINT8_MAX;
		bool useMultiThreading = true;

		BoolAtom runControl = true;
		std::vector<std::jthread> threads;

		std::condition_variable signaler;
		std::array<Allocator::BlockingQueue<std::pair<std::coroutine_handle<>, PromiseBase*>>, 3> taskQueues;

		constexpr void ResizeThreads(U8 oldCount, U8 currentCount) noexcept;
		bool AddThread(U8 threadId) noexcept;
		void Worker(std::stop_token& stoken) noexcept;

	public:
		ThreadPool() noexcept;
		ZE_CLASS_MOVE(ThreadPool);
		~ThreadPool() { Stop(); }

		static constexpr VendorCPU GetCurrentCPU() noexcept { return currentCPU; }
		static constexpr U8 GetCoresCount() noexcept { return coresCount; }
		static constexpr U8 GetLogicalCoresCount() noexcept { return logicalCoresCount; }

		template <typename Func, typename... Args>
		static constexpr Expected<std::jthread> CreatePersistentThread(Func&& f, Args&&... args) noexcept;

		template <typename Func, typename... Args>
		constexpr auto Schedule(ThreadPriority priority, Func&& f, Args&&... args) noexcept;

		constexpr U8 GetWorkerThreadsCount() const noexcept;
		constexpr void ResetThreadsCount() noexcept;
		constexpr void ClampThreadsCount(U8 maxThreads) noexcept;
		constexpr void SetSMT(bool val) noexcept;
		// allocThreadsCount: decrease threadpool count by X for static threads that will not be managed by this pool
		// customThreadCount: set custom override to number of threads (no function will change this number)
		constexpr void Init(U8 allocThreadsCount = 0, U8 customThreadCount = 0) noexcept;

		// Get some job from thread pool if available
		bool GetNextJob(std::coroutine_handle<>& handle, PromiseBase*& promise) noexcept;
		void Stop() noexcept;
	};

#pragma region Functions
	template <typename Func, typename... Args>
	constexpr Expected<std::jthread> ThreadPool::CreatePersistentThread(Func&& f, Args&&... args) noexcept
	{
		Status result = {};
		auto handleFail = [&](Status code)
			{
				std::string_view msg = "Failed to start new persistent thread!";
				if (code)
				{
					ZE_CODE_ERROR(code, msg);
				}
				else
					Logger::Error(msg);
				result = code;
			};

		try
		{
			std::jthread thread(std::forward<Func>(f), std::forward<Args>(args)...);
			return thread;
		}
		catch (const std::system_error& e)
		{
			handleFail(e.code());
		}
		catch (const std::exception& e)
		{
			Logger::Error(e.what());
			handleFail(std::make_error_code(std::errc::invalid_argument));
		}
		catch (...)
		{
			handleFail(std::make_error_code(std::errc::invalid_argument));
		}
		return std::unexpected(result);
	}

	template <typename Func, typename... Args>
	constexpr auto ThreadPool::Schedule(ThreadPriority priority, Func&& f, Args&&... args) noexcept
	{
		using RawResult = std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>;
		using TaskType = to_task_t<RawResult>;

		TaskType task(std::forward<Func>(f), std::forward<Args>(args)...);
		if (GetWorkerThreadsCount() > 0 && runControl)
		{
			task.SetThreadPoolOwnership();
			taskQueues[static_cast<U8>(priority)].EmplaceBack(task.GetHandle(), &task.GetHandle().promise());
			signaler.notify_one();
		}
		else
			task.GetHandle().resume();
		return task;
	}

	constexpr U8 ThreadPool::GetWorkerThreadsCount() const noexcept
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
		return Math::Clamp(static_cast<U8>(count - allocatedThreads - 1), static_cast<U8>(0), maxThreadsCount);
	}

	constexpr void ThreadPool::ResetThreadsCount() noexcept
	{
		const U8 oldCount = GetWorkerThreadsCount();
		maxThreadsCount = UINT8_MAX;
		ResizeThreads(oldCount, GetWorkerThreadsCount());
	}

	constexpr void ThreadPool::ClampThreadsCount(U8 maxThreads) noexcept
	{
		const U8 oldCount = GetWorkerThreadsCount();
		maxThreadsCount = maxThreads;
		ResizeThreads(oldCount, GetWorkerThreadsCount());
	}

	constexpr void ThreadPool::SetSMT(bool val) noexcept
	{
		const U8 oldCount = GetWorkerThreadsCount();
		useMultiThreading = val;
		ResizeThreads(oldCount, GetWorkerThreadsCount());
	}

	constexpr void ThreadPool::Init(U8 allocThreadsCount, U8 customThreadCount) noexcept
	{
		threadsCountOverride = customThreadCount;
		if (customThreadCount != UINT8_MAX)
		{
			allocatedThreads = allocThreadsCount;

			// Create worker threads that will sleep waiting for new job to execute
			const U8 count = GetWorkerThreadsCount();
			threads.reserve(count);

			for (U8 i = 0; i < count; ++i)
				if (AddThread(i))
					break;
		}
	}
#pragma endregion
}
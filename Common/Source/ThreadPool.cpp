#include "ThreadPool.h"

namespace ZE
{
	constexpr void ThreadPool::ResizeThreads(U8 oldCount, U8 currentCount) noexcept
	{
		if (currentCount < oldCount)
		{
			for (U8 i = currentCount; i < oldCount; ++i)
				threads[i].request_stop();
			signaler.notify_all();
			threads.resize(currentCount);
		}
		else
		{
			for (; oldCount < currentCount; ++oldCount)
				if (AddThread(oldCount))
					break;
		}
	}

	bool ThreadPool::AddThread(U8 threadId) noexcept
	{
		bool fail = false;
		auto handleFail = [&](Status code)
			{
				std::string_view msg = "Failed to start new worker thread, capping thread count to " + std::to_string(threadId);
				if (code)
				{
					ZE_CODE_ERROR(code, msg);
				}
				else
					Logger::Error(msg);
				fail = true;
				threadsCountOverride = threadId == 0 ? UINT8_MAX : threadId;
			};

		try
		{
			threads.emplace_back([this](std::stop_token stoken) { Worker(stoken); });
		}
		catch (const std::system_error& e)
		{
			handleFail(e.code());
		}
		catch (const std::exception& e)
		{
			Logger::Error(e.what());
			handleFail({});
		}
		catch (...)
		{
			handleFail({});
		}
		return fail;
	}

	void ThreadPool::Worker(std::stop_token& stoken) noexcept
	{
		// Process tasks till stopped by master thread
		while (true)
		{
			bool newItem = false;
			std::coroutine_handle<> handle = nullptr;
			PromiseBase* promise = nullptr;

			// Creating local mutex because access to task queue is protected by more efficient one
			std::mutex mutex;
			std::unique_lock lock(mutex);
			// Wait for new task and try obtain it (more important jobs first)
			signaler.wait(lock, [this, &stoken, &newItem, &handle, &promise]() noexcept -> bool
				{
					newItem = GetNextJob(handle, promise);
					return newItem || stoken.stop_requested() || !runControl;
				});

			// Don't stop when task queue is not empty
			if (!newItem && (stoken.stop_requested() || !runControl))
				return;
			
			// Check if task can be started, otherwise skip
			if (promise->TryClaimExecution())
				handle.resume();
			if (handle.done())
				promise->DecrementRef();
		}
	}

	ThreadPool::ThreadPool() noexcept
	{
		// CPU cores detection
		// AMD recomendations https://github.com/GPUOpen-LibrariesAndSDKs/cpu-core-counts/blob/master/windows/AMDCoreCount.cpp
		// Intel guide https://www.intel.com/content/www/us/en/developer/articles/guide/12th-gen-intel-core-processor-gamedev-guide.html
		if (coresCount == 0)
		{
			// Check CPU vendor
			U32 eax, ebx, ecx, edx;
			Intrin::CPUID(eax, ebx, ecx, edx, 0);
			char vendor[13];
			*reinterpret_cast<U32*>(vendor) = ebx;
			*reinterpret_cast<U32*>(vendor + 4) = edx;
			*reinterpret_cast<U32*>(vendor + 8) = ecx;
			vendor[12] = '\0';

			// Check CPU family
			Intrin::CPUID(eax, ebx, ecx, edx, 1);
			const U8 family = (eax >> 8) & 0x0F;
			const U8 extendedFamily = (eax >> 20) & 0xFF;

			if (std::strcmp(vendor, "AuthenticAMD") == 0)
			{
				// Check for Bulldozer family CPUs and older
				const U8 displayFamily = family != 0x0F ? family : (extendedFamily + family);
				if (displayFamily < 0x17)
					currentCPU = VendorCPU::AMDOld;
				else
					currentCPU = VendorCPU::AMD;

				Intrin::CPUID(eax, ebx, ecx, edx, 0x80000008);

				const U8 coresIdSize = (ecx >> 12) & 0x0F;
				if (coresIdSize == 0)
					logicalCoresCount = (ecx & 0xFF) + 1;
				else
					logicalCoresCount = Utils::SafeCast<U8>(std::pow(2U, coresIdSize));

				Intrin::CPUID(eax, ebx, ecx, edx, 0x8000001E);
				coresCount = logicalCoresCount / (((ebx >> 8) & 0xFF) + 1);
			}
			else if (std::strcmp(vendor, "GenuineIntel") == 0)
			{
				// Check for new hybrid Intel CPUs
				Intrin::CPUIDEX(eax, ebx, ecx, edx, 7, 0);
				if (edx & 0x8000)
				{
					// Currently hard to detect correctly without OS dependet code
					currentCPU = VendorCPU::IntelHybrid;
					coresCount = logicalCoresCount = Utils::SafeCast<U8>(std::thread::hardware_concurrency());
				}
				else
				{
					currentCPU = VendorCPU::Intel;

					Intrin::CPUIDEX(eax, ebx, ecx, edx, 0x0B, 1);
					logicalCoresCount = ebx & 0x0F;

					Intrin::CPUIDEX(eax, ebx, ecx, edx, 0x0B, 0);
					const U8 threadsPerCore = ebx & 0x0F;
					coresCount = logicalCoresCount / threadsPerCore;
				}
			}
			else
				coresCount = logicalCoresCount = Utils::SafeCast<U8>(std::thread::hardware_concurrency());
		}
	}

	bool ThreadPool::GetNextJob(std::coroutine_handle<>& handle, PromiseBase*& promise) noexcept
	{
		std::pair<std::coroutine_handle<>, PromiseBase*> task;
		for (auto& queue : taskQueues)
		{
			if (queue.TryPopFront(task))
			{
				handle = task.first;
				promise = task.second;
				return true;
			}
		}
		return false;
	}

	void ThreadPool::Stop() noexcept
	{
		runControl = false;
		signaler.notify_all();
		threads.clear();
	}
}
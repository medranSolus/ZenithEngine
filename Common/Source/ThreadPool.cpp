#include "ThreadPool.h"
#include "Intrinsics.h"

namespace ZE
{
	constexpr void ThreadPool::ResizeThreads(U8 oldCount, U8 currentCount) noexcept
	{
		for (; oldCount < currentCount; ++oldCount)
		{
			threadRunControls[oldCount] = true;
			threads.emplace_back(&ThreadPool::Worker, this, std::cref(threadRunControls[oldCount]));
		}
		if (currentCount < oldCount)
		{
			for (U8 i = currentCount; i < oldCount; ++i)
				threadRunControls[i] = false;
			signaler.notify_all();
			for (U8 i = currentCount; i < oldCount; ++i)
				threads[i].join();
			threads.resize(currentCount);
		}
	}

	void ThreadPool::Worker(const BoolAtom& run) const noexcept
	{
		// Process tasks till stopped by master thread
		while (true)
		{
			bool newItem = false;
			std::function<void()> task;

			// Creating local mutex because access to task queue is protected by more efficient one
			std::mutex mutex;
			std::unique_lock lock(mutex);
			// Wait for new task and try obtain it (more important jobs first)
			signaler.wait(lock, [this, &run, &newItem, &task]() -> bool
				{
					for (auto& queue : taskQueues)
					{
						newItem = queue.TryPopFront(task);
						if (newItem)
							break;
					}
					return newItem || !(run && runControl);
				});

			// Don't stop when task queue is not empty
			if ((run || runControl) && !newItem)
				return;
			task();
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

			if (strcmp(vendor, "AuthenticAMD") == 0)
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
			else if (strcmp(vendor, "GenuineIntel") == 0)
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

	ThreadPool::~ThreadPool()
	{
		runControl = false;
		signaler.notify_all();
		for (std::thread& worker : threads)
			worker.join();
		if (threadRunControls)
			threadRunControls.DeleteArray();
	}
}
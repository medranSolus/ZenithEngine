#include "Platform/WinAPI/AsyncBackgroundThread.h"
#include "Platform/WinAPI/IocpOverlapped.h"

namespace ZE::Platform::WinAPI
{
	void AsyncBackgroundThread::IoWorker(std::stop_token& stoken) const noexcept
	{
		if (iocp == nullptr)
		{
			ZE_FAIL("IOCP not yet initialized!");
			return;
		}

		// Automatic wake-up from kernel wait
		std::stop_callback stopCallback(stoken, []() { PostQueuedCompletionStatus(iocp, 0, SHUTDOWN_KEY, nullptr); });

		while (!stoken.stop_requested())
		{
			DWORD bytesTransferred = 0;
			ULONG_PTR completionKey = 0;
			LPOVERLAPPED overlapped = nullptr;

			BOOL success = GetQueuedCompletionStatus(iocp, &bytesTransferred, &completionKey, &overlapped, INFINITE);

			if (completionKey == SHUTDOWN_KEY)
			{
				// Propagete in case of more threads
				PostQueuedCompletionStatus(iocp, 0, SHUTDOWN_KEY, nullptr);
				break;
			}

			if (overlapped != nullptr)
			{
				IocpOverlapped* iocpOverlapped = static_cast<IocpOverlapped*>(overlapped);

				iocpOverlapped->BytesTransferred = bytesTransferred;
				iocpOverlapped->Error = success ? Status{} : ZE_WIN_LAST_ERROR();

				if (iocpOverlapped->Continuation)
				{
					// Enqueue anything that was being executed before
					pool->Schedule(ThreadPriority::High, [h = iocpOverlapped->Continuation]() noexcept { h.resume(); });
				}
			}
		}

		CloseHandle(iocp);
		iocp = nullptr;
	}

	Status AsyncBackgroundThread::RegisterFileHandle(HANDLE file) noexcept
	{
		if (iocp == nullptr)
		{
			ZE_FAIL("IOCP not yet initialized!");

			return std::make_error_code(std::errc::invalid_argument);
		}

		if (CreateIoCompletionPort(file, iocp, reinterpret_cast<ULONG_PTR>(file), 0) != iocp)
			return ZE_WIN_LAST_ERROR();
		SetFileCompletionNotificationModes(file, FILE_SKIP_COMPLETION_PORT_ON_SUCCESS);
		return {};
	}

	Status AsyncBackgroundThread::Start(ThreadPool& threads, U8 iocpThreadsCount) noexcept
	{
		if (iocp == nullptr)
		{
			iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, iocpThreadsCount);
			if (iocp == nullptr)
				return ZE_WIN_LAST_ERROR();

			pool = &threads;
			ZE_EXPECT_RET_FAILED_CODE(ioWorkThread, ThreadPool::CreatePersistentThread([this](std::stop_token stoken) { IoWorker(stoken); }));
		}
		return {};
	}
}
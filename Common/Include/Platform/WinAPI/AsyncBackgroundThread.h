#pragma once
#include "ThreadPool.h"
#include "WinAPI.h"

namespace ZE::Platform::WinAPI
{
	class AsyncBackgroundThread final
	{
		static constexpr ULONG_PTR SHUTDOWN_KEY = static_cast<ULONG_PTR>(-1);

		static inline HANDLE iocp = nullptr;
		ThreadPool* pool = nullptr;
		std::jthread ioWorkThread;

		void IoWorker(std::stop_token& stoken) const noexcept;

	public:
		AsyncBackgroundThread() = default;
		ZE_CLASS_MOVE(AsyncBackgroundThread);
		~AsyncBackgroundThread() = default;

		static Status RegisterFileHandle(HANDLE file) noexcept;

		void Stop() noexcept { ioWorkThread.request_stop(); }

		Status Start(ThreadPool& threads, U8 iocpThreadsCount) noexcept;
	};
}
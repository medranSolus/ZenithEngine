#pragma once
#include "Macros.h"

#if _ZE_PLATFORM_WINDOWS
#include "Platform/WinAPI/AsyncBackgroundThread.h"
namespace ZE { typedef Platform::WinAPI::AsyncBackgroundThread PlatformAsyncBackgroundThread; }
#else
#	error Missing AsyncBackgroundThread platform specific implementation!
#endif

namespace ZE::IO
{
	// Facility for receiving async I/O completion requests
	class AsyncBackgroundThread final
	{
		PlatformAsyncBackgroundThread platformImpl;
		
	public:
		AsyncBackgroundThread() = default;
		ZE_CLASS_MOVE(AsyncBackgroundThread);
		~AsyncBackgroundThread() = default;

		// Allow for execution of IOCP requests in the OS
		Status Start(ThreadPool& threads, U8 iocpThreadsCount = 0) noexcept { return platformImpl.Start(threads, iocpThreadsCount); }
		void Stop() noexcept { platformImpl.Stop(); }
	};
}
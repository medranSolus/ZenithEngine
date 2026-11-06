#include "SFX/StreamOrderAwaiter.h"

namespace ZE::SFX
{
	void StreamOrderData::AbortAllWaiters() noexcept
	{
		WaitLock.Lock();
		auto localWaits = WaitTable;
		WaitTable = {};
		WaitLock.Unlock();

		for (auto h : localWaits)
		{
			if (h)
				h.resume();
		}
	}

	void StreamOrderData::PerformNextWaitingTask(U64 taskIdx) noexcept
	{
		WaitLock.Lock();
		std::coroutine_handle<> h = WaitTable.at(taskIdx % SFX::StreamState::BUFFER_COUNT);
		WaitTable.at(taskIdx % SFX::StreamState::BUFFER_COUNT) = nullptr;
		WaitLock.Unlock();

		if (h)
			h.resume();
	}

	void StreamOrderData::FinishNextBuffer(bool advanceSubmission) noexcept
	{
		if (advanceSubmission)
			++NextSubmitBufferIdx;
		++FinishedBuffers;
		FinishedBuffers.notify_one();
	}

	void StreamOrderData::FlushBuffers() noexcept
	{
		Alive = false;
		AbortAllWaiters();
		NextSubmitBufferIdx = 0;
	}

	void StreamOrderData::WaitAllBuffersFinished() noexcept
	{
		U8 awaitingBuffers = 0;
		do
		{
			FinishedBuffers.wait(awaitingBuffers);
			awaitingBuffers = FinishedBuffers;
		} while (awaitingBuffers < SFX::StreamState::BUFFER_COUNT);
		FinishedBuffers = 0;
		Alive = true;
	}

	bool StreamOrderAwaiter::await_ready() const noexcept
	{
		return Data->NextSubmitBufferIdx.load(std::memory_order_acquire) >= BufferIdx
			|| !Data->Alive.load(std::memory_order_acquire);
	}

	bool StreamOrderAwaiter::await_suspend(std::coroutine_handle<> h) noexcept
	{
		Handle = h;
		Data->WaitLock.Lock();

		// Re-check condition under lock
		if (Data->NextSubmitBufferIdx.load(std::memory_order_relaxed) >= BufferIdx
			|| !Data->Alive.load(std::memory_order_relaxed))
		{
			Data->WaitLock.Unlock();
			return false;
		}

		// Register into the fixed table slot
		Data->WaitTable.at(BufferIdx % SFX::StreamState::BUFFER_COUNT) = h;

		Data->WaitLock.Unlock();
		return true;
	}

	bool StreamOrderAwaiter::await_resume() const noexcept
	{
		return Data->Alive.load(std::memory_order_acquire);
	}

}
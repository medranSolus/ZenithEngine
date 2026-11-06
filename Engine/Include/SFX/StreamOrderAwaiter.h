#pragma once

namespace ZE::SFX
{
	// Data needed to manage streaming buffers ordering
	struct StreamOrderData
	{
		BoolAtom Alive = true;
		UA8 FinishedBuffers = 0;
		UA64 NextSubmitBufferIdx = 0;

		std::array<std::coroutine_handle<>, SFX::StreamState::BUFFER_COUNT> WaitTable;
		SpinLock WaitLock;

		// Gracefully finish all waits
		void AbortAllWaiters() noexcept;
		// Advance and resume any awaiting tasks if any
		void PerformNextWaitingTask(U64 taskIdx) noexcept;
		// Indicate that current buffer is finished and won't submit any more audio data to the stream
		void FinishNextBuffer(bool advanceSubmission) noexcept;
		// Pause stream and force all buffers to drop execution
		void FlushBuffers() noexcept;
		// Wait quickly till all buffers "ended" after flush and resume the stream
		void WaitAllBuffersFinished() noexcept;
	};

	// Awaiter helping in ordering buffer submissions to the streaming objects
	struct StreamOrderAwaiter
	{
		Ptr<StreamOrderData> Data;
		U64 BufferIdx;
		std::coroutine_handle<> Handle = {};

		bool await_ready() const noexcept;
		bool await_suspend(std::coroutine_handle<> h) noexcept;
		bool await_resume() const noexcept;
	};
}
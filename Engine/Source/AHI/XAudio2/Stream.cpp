#include "AHI/XAudio2/Stream.h"

namespace ZE::AHI::XAudio2
{
	bool Stream::StreamData::Awaiter::await_ready() const noexcept
	{
		return Data->NextSubmitBufferIdx.load(std::memory_order_acquire) >= BufferIdx
			|| !Data->Alive.load(std::memory_order_acquire);
	}

	bool Stream::StreamData::Awaiter::await_suspend(std::coroutine_handle<> h) noexcept
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

	bool Stream::StreamData::Awaiter::await_resume() const noexcept
	{
		return Data->Alive.load(std::memory_order_acquire);
	}

	Stream::StreamData::~StreamData()
	{
		AbortAllWaiters();
		if (SourceVoice)
			SourceVoice->DestroyVoice();
	}

	void Stream::StreamData::AbortAllWaiters() noexcept
	{
		// Gracefully finish all waits
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

	void Stream::StreamCallback::OnBufferEnd(void* bufferCtx) noexcept
	{
		if (!data->Alive)
		{
			// In case of rewind position call
			++data->FinishedBuffers;
			data->FinishedBuffers.notify_one();
			return;
		}

		Settings::GetThreadPool().Schedule(ThreadPriority::High,
			[](std::shared_ptr<StreamData> data, U64 bufferIdx) noexcept -> Task<void>
			{
				ZE_ASSERT(data->NextSubmitBufferIdx <= bufferIdx, "Buffer has been submitted twice!");

				auto endSubmission = [&data]() noexcept -> void
					{
						++data->NextSubmitBufferIdx;
						++data->FinishedBuffers;
						data->FinishedBuffers.notify_one();
					};

				// Wait with reading till it's this buffer turn or return early if stream is wrapping up
				if (!co_await StreamData::Awaiter{ data.get(), bufferIdx })
					co_return endSubmission();

				U32 audioSize = data->StateInfo.GetBlockSize();
				U8* writeRegion = data->RingBuffer.get() + data->StateInfo.GetBlockSize() * (bufferIdx % SFX::StreamState::BUFFER_COUNT);
				Status stat = co_await data->StateInfo.ReadNextChunk(audioSize, writeRegion, data->EnableLoop);
				if (stat)
				{
					if (IO::EofResult::IsEOF(stat) && !data->EnableLoop)
					{
						audioSize = IO::EofResult::GetRealBytes(stat);
						// Drop execution if stream has already ended
						if (audioSize == 0)
							co_return endSubmission();
					}
					else
					{
						ZE_CODE_ERROR(stat, "Error reading next buffer in the stream! Buffer index: " + std::to_string(bufferIdx));
						co_return endSubmission();
					}
				}
				else
				{
					// Prepare buffer submission
					XAUDIO2_BUFFER buffer = {};
					buffer.Flags = !data->EnableLoop && data->StateInfo.IsStreamEnd() ? XAUDIO2_END_OF_STREAM : 0;
					buffer.AudioBytes = audioSize;
					buffer.pAudioData = writeRegion;
					buffer.PlayBegin = 0;
					buffer.PlayLength = 0;
					buffer.LoopBegin = 0;
					buffer.LoopLength = 0;
					buffer.LoopCount = 0;
					buffer.pContext = reinterpret_cast<void*>(bufferIdx + SFX::StreamState::BUFFER_COUNT); // Set which next buffer to load in this region

					HRESULT hr = data->SourceVoice->SubmitSourceBuffer(&buffer, nullptr);
					if (FAILED(hr))
					{
						ZE_CODE_ERROR(ZE_XA2_ERROR(hr), "Error submitting buffer to the stream! Buffer index: " + std::to_string(bufferIdx));
					}

					// Indicate for others that stream has ended
					if (buffer.Flags)
						data->Alive = false;
				}
				const U64 nextTask = ++data->NextSubmitBufferIdx;

				// Last check if should ignore everything and exit
				if (!data->Alive)
					co_return;

				// Advance and resume any awaiting tasks if any
				data->WaitLock.Lock();
				std::coroutine_handle<> h = data->WaitTable.at(nextTask % SFX::StreamState::BUFFER_COUNT);
				data->WaitTable.at(nextTask % SFX::StreamState::BUFFER_COUNT) = nullptr;
				data->WaitLock.Unlock();

				if (h)
					h.resume();
				co_return;
			}, data, reinterpret_cast<U64>(bufferCtx));
	}

	Status Stream::SubmitInitialBuffers() noexcept
	{		
		XAUDIO2_BUFFER buffer = {};
		buffer.Flags = 0;
		buffer.AudioBytes = data->StateInfo.GetBlockSize();
		buffer.pAudioData = data->RingBuffer.get();
		buffer.PlayBegin = 0;
		buffer.PlayLength = 0;
		buffer.LoopBegin = 0;
		buffer.LoopLength = 0;
		buffer.LoopCount = 0;
		for (U8 i = 0; i < SFX::StreamState::BUFFER_COUNT; ++i)
		{
			buffer.pContext = reinterpret_cast<void*>(i);
			ZE_XA2_RET_FAILED(data->SourceVoice->SubmitSourceBuffer(&buffer, nullptr));
			buffer.pAudioData += data->StateInfo.GetBlockSize();
		}
		return {};
	}

	Stream::~Stream()
	{
		if (data)
		{
			data->Alive = false;
			if (data->SourceVoice)
				Pause();
		}
	}

	Expected<Stream> Stream::Create(SFX::Device& dev, const SFX::StreamDesc& desc) noexcept
	{
		Stream audioStream;
		audioStream.data = std::make_shared<StreamData>();

		// Get basic info about audio file
		SFX::AudioDesc audioInfo = {};
		ZE_EXPECT_RET_FAILED(audioStream.data->StateInfo, SFX::StreamState::Create(*desc.FileHandle.get(),
			desc.FileStart, desc.RegionSize, desc.SourceType, audioInfo));

		audioStream.data->EnableLoop = desc.LoopAudio;
		audioStream.data->FileHandle = desc.FileHandle;
		audioStream.data->RingBuffer = std::make_unique_for_overwrite<U8[]>(audioStream.data->StateInfo.GetBlockSize() * SFX::StreamState::BUFFER_COUNT);
		audioStream.callback = std::make_unique<StreamCallback>(audioStream.data);
		ZE_EXPECT_RET_FAILED(audioStream.data->SourceVoice, CreateSourceVoice(dev.Get().xa2.GetDevice(), audioInfo, desc.Group ? desc.Group->Get().xa2.GetVoice() : nullptr, audioStream.callback.get()));

		// Read whole initial data (blocking call)
		Status stat = audioStream.data->StateInfo.ReadNextChunk(audioStream.data->StateInfo.GetBlockSize() * SFX::StreamState::BUFFER_COUNT,
			audioStream.data->RingBuffer.get(), desc.LoopAudio).Get();
		if (stat)
		{
			if (IO::EofResult::IsEOF(stat))
			{
				ZE_FAIL("Buffer too small to be efficiently used as a SFX::Stream! Switch to simpler SFX::Track for this sound file!");
			}
			return std::unexpected(stat);
		}

		ZE_CODE_RET_FAILED_EXPECT(audioStream.SubmitInitialBuffers());
		return audioStream;
	}

	Task<Status> Stream::ResetPosition(bool autoPlay, bool enableLoop) noexcept
	{
		ZE_ASSERT(data, "Not initialized!");
		Pause();
		data->Alive = false;
		data->AbortAllWaiters();
		data->NextSubmitBufferIdx = 0;
		data->EnableLoop = enableLoop;
		ZE_XA2_RET_FAILED_CORO(data->SourceVoice->FlushSourceBuffers());

		// Rewind and load initial data
		ZE_CODE_RET_FAILED_CORO(data->StateInfo.RewindToBegining(true));
		ZE_CODE_RET_FAILED_CORO(co_await data->StateInfo.ReadNextChunk(data->StateInfo.GetBlockSize() * SFX::StreamState::BUFFER_COUNT, data->RingBuffer.get(), data->EnableLoop));

		// Wait quickly till all buffers "ended" after flush
		U8 awaitingBuffers = 0;
		do
		{
			data->FinishedBuffers.wait(awaitingBuffers);
			awaitingBuffers = data->FinishedBuffers;
		} while (awaitingBuffers < SFX::StreamState::BUFFER_COUNT);
		data->FinishedBuffers = 0;
		data->Alive = true;
		
		ZE_CODE_RET_FAILED_CORO(SubmitInitialBuffers());
		if (autoPlay)
			Resume();
		co_return Status{};
	}
}
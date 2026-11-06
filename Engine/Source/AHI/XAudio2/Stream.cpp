#include "AHI/XAudio2/Stream.h"

namespace ZE::AHI::XAudio2
{
	Stream::StreamData::~StreamData()
	{
		OrderData.AbortAllWaiters();
		if (SourceVoice)
			SourceVoice->DestroyVoice();
	}

	void Stream::StreamCallback::OnBufferEnd(void* bufferCtx) noexcept
	{
		if (!data->OrderData.Alive)
		{
			// In case of rewind position call
			data->OrderData.FinishNextBuffer(false);
			return;
		}

		Settings::GetThreadPool().Schedule(ThreadPriority::High,
			[](std::shared_ptr<StreamData> data, U64 bufferIdx) noexcept -> Task<void>
			{
				ZE_ASSERT(data->OrderData.NextSubmitBufferIdx <= bufferIdx, "Buffer has been submitted twice!");

				// Wait with reading till it's this buffer turn or return early if stream is wrapping up
				if (!co_await SFX::StreamOrderAwaiter{ &data->OrderData, bufferIdx })
					co_return data->OrderData.FinishNextBuffer(true);

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
							co_return data->OrderData.FinishNextBuffer(true);
					}
					else
					{
						ZE_CODE_ERROR(stat, "Error reading next buffer in the stream! Buffer index: " + std::to_string(bufferIdx));
						co_return data->OrderData.FinishNextBuffer(true);
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
						data->OrderData.Alive = false;
				}
				const U64 nextTask = ++data->OrderData.NextSubmitBufferIdx;

				// Last check if should ignore everything and exit
				if (!data->OrderData.Alive)
					co_return;

				co_return data->OrderData.PerformNextWaitingTask(nextTask);
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
			data->OrderData.Alive = false;
			if (data->SourceVoice)
			{
				Status stat = Pause();
				if (stat)
				{
					ZE_CODE_ERROR(stat, "Error stopping Stream voice!");
				}
			}
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
			ZE_ASSERT(!IO::EofResult::IsEOF(stat), "Buffer too small to be efficiently used as a SFX::Stream! Switch to simpler SFX::Track for this sound file!");
			return std::unexpected(stat);
		}

		ZE_CODE_RET_FAILED_EXPECT(audioStream.SubmitInitialBuffers());
		return audioStream;
	}

	Task<Status> Stream::ResetPosition(bool autoPlay, bool enableLoop) noexcept
	{
		ZE_ASSERT(data, "Not initialized!");
		ZE_CODE_RET_FAILED_CORO(Pause());

		data->OrderData.FlushBuffers();
		data->EnableLoop = enableLoop;
		ZE_XA2_RET_FAILED_CORO(data->SourceVoice->FlushSourceBuffers());

		// Rewind and load initial data
		ZE_CODE_RET_FAILED_CORO(data->StateInfo.RewindToBegining(true));
		ZE_CODE_RET_FAILED_CORO(co_await data->StateInfo.ReadNextChunk(data->StateInfo.GetBlockSize() * SFX::StreamState::BUFFER_COUNT, data->RingBuffer.get(), data->EnableLoop));

		data->OrderData.WaitAllBuffersFinished();
		
		ZE_CODE_RET_FAILED_CORO(SubmitInitialBuffers());
		co_return autoPlay ? Resume() : Status{};
	}
}
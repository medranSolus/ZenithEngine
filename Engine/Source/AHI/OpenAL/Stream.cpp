#include "AHI/OpenAL/Stream.h"

namespace ZE::AHI::OpenAL
{
	Stream::StreamData::~StreamData()
	{
		OrderData.AbortAllWaiters();
		if (Source != UINT_MAX)
		{
			alDeleteSources(1, &Source);
			ZE_AL_CHECK("Error deleting stream source!");
		}
	}

	Stream::~Stream()
	{
		if (data)
			data->OrderData.Alive = false;
		if (source != UINT_MAX)
		{
			Status stat = Pause();
			if (stat)
			{
				ZE_CODE_ERROR(stat, "Error stopping Stream voice!");
			}
			auto it = activeStreams.find(source);
			if (it != activeStreams.end())
				activeStreams.erase(it);
		}
	}

	Expected<Stream> Stream::Create(SFX::Device& dev, const SFX::StreamDesc& desc) noexcept
	{
		auto& device = dev.Get().oal;
		Stream audioStream;
		audioStream.data = std::make_shared<StreamData>();
		audioStream.data->EnableLoop = desc.LoopAudio;
		audioStream.data->FileHandle = desc.FileHandle;

		// Register new object for streaming
		alGenSources(1, &audioStream.source);
		ZE_AL_RET_FAILED_EXPECT();
		ZE_AL_SET_DEBUG_ID(device, AL_SOURCE_EXT, audioStream.source, "Stream source");

		ZE_CODE_RET_FAILED_EXPECT(SetSource2D(device, audioStream.source));
		audioStream.data->Source = audioStream.source;

		ZE_ASSERT(!activeStreams.contains(audioStream.source), "Stream source already registered!");
		activeStreams.emplace(audioStream.source, audioStream.data);

		// Get basic info about audio file
		SFX::AudioDesc audioInfo = {};
		ZE_EXPECT_RET_FAILED(audioStream.data->StateInfo, SFX::StreamState::Create(*desc.FileHandle.get(), desc.FileStart, desc.RegionSize, desc.SourceType, audioInfo));

		// Read whole initial data (blocking call)
		audioStream.data->RingBuffer = std::make_unique_for_overwrite<U8[]>(audioStream.data->StateInfo.GetBlockSize() * SFX::StreamState::BUFFER_COUNT);
		Status stat = audioStream.data->StateInfo.ReadNextChunk(audioStream.data->StateInfo.GetBlockSize() * SFX::StreamState::BUFFER_COUNT,
			audioStream.data->RingBuffer.get(), desc.LoopAudio).Get();
		if (stat)
		{
			ZE_ASSERT(!IO::EofResult::IsEOF(stat), "Buffer too small to be efficiently used as a SFX::Stream! Switch to simpler SFX::Track for this sound file!");
			return std::unexpected(stat);
		}

		// Pass data into the sound buffers
		alGenBuffers(SFX::StreamState::BUFFER_COUNT, &audioStream.data->Buffers.front());
		ZE_AL_RET_FAILED_EXPECT();
		const ALenum format = GetALFormat(device, audioInfo.Channels, audioInfo.BitsPerSample, audioInfo.IsFloat);
		for (U8 i = 0; i < SFX::StreamState::BUFFER_COUNT; ++i)
		{
			ZE_AL_SET_DEBUG_ID(device, AL_BUFFER_EXT, audioStream.data->Buffers.at(i), "Stream buffer " + std::to_string(i));

			alBufferDataStatic(audioStream.data->Buffers.at(i), format,
				audioStream.data->RingBuffer.get() + i * audioStream.data->StateInfo.GetBlockSize(),
				audioStream.data->StateInfo.GetBlockSize(), audioInfo.SampleRate);
			ZE_AL_RET_FAILED_EXPECT();
		}

		// Setup initial buffers
		alSourceQueueBuffers(audioStream.source, SFX::StreamState::BUFFER_COUNT, &audioStream.data->Buffers.front());
		ZE_AL_RET_FAILED_EXPECT();

		return audioStream;
	}

	Task<Status> Stream::ResetPosition(bool autoPlay, bool enableLoop) noexcept
	{
		ZE_ASSERT(data, "Not initialized!");

		data->OrderData.FlushBuffers();
		data->EnableLoop = enableLoop;
		data->NextLoadBufferIdx = 0;

		// Flush any buffer data
		alSourceStop(source);
		ZE_AL_RET_FAILED_CORO();
		ALint queuedBuffers = 0;
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &queuedBuffers);
		ZE_AL_RET_FAILED_CORO();
		std::vector<ALuint> buffers(queuedBuffers);
		alSourceUnqueueBuffers(source, queuedBuffers, buffers.data());
		ZE_AL_RET_FAILED_CORO();

		// Wait quickly till all buffers in flight have stopped their work
		U8 awaitingBuffers = SFX::StreamState::BUFFER_COUNT;
		do
		{
			data->BuffersInFlight.wait(awaitingBuffers);
			awaitingBuffers = data->BuffersInFlight;
		} while (awaitingBuffers > 0);
		data->OrderData.Alive = true;

		// Rewind and load initial data
		ZE_CODE_RET_FAILED_CORO(data->StateInfo.RewindToBegining(true));
		ZE_CODE_RET_FAILED_CORO(co_await data->StateInfo.ReadNextChunk(data->StateInfo.GetBlockSize() * SFX::StreamState::BUFFER_COUNT, data->RingBuffer.get(), data->EnableLoop));

		// Send new buffers out
		alSourceQueueBuffers(source, SFX::StreamState::BUFFER_COUNT, &data->Buffers.front());
		ZE_AL_RET_FAILED_CORO();
		co_return autoPlay ? Resume() : Status{};
	}

	void Stream::OnBufferEndCallback(ALuint source, U8 buffersFinished) noexcept
	{
		auto it = activeStreams.find(source);
		if (it != activeStreams.end())
		{
			++it->second->BuffersInFlight;
			if (!it->second->OrderData.Alive)
			{
				// In case of rewind position call
				it->second->BuffersInFlight.fetch_sub(1);
				it->second->BuffersInFlight.notify_all();
				return;
			}

			for (U8 i = 0; i < buffersFinished; ++i)
			{
				Settings::GetThreadPool().Schedule(ThreadPriority::High,
					[](std::shared_ptr<StreamData> data, U64 bufferIdx) noexcept -> Task<void>
					{
						auto endSubmission = [&data]() -> void
							{
								data->BuffersInFlight.fetch_sub(1);
								data->BuffersInFlight.notify_all();
							};

						// Wait with reading till it's this buffer turn or return early if stream is wrapping up
						if (!co_await SFX::StreamOrderAwaiter{ &data->OrderData, bufferIdx })
						{
							++data->OrderData.NextSubmitBufferIdx;
							co_return endSubmission();
						}

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
								{
									++data->OrderData.NextSubmitBufferIdx;
									co_return endSubmission();
								}
							}
							else
							{
								ZE_CODE_ERROR(stat, "Error reading next buffer in the stream! Buffer index: " + std::to_string(bufferIdx));
								++data->OrderData.NextSubmitBufferIdx;
								co_return endSubmission();
							}
						}
						else
						{
							ALuint buffer = UINT_MAX;
							alSourceUnqueueBuffers(data->Source, 1, &buffer);
							ALenum error = alGetError();
							if (error == AL_NO_ERROR)
							{
								ZE_ASSERT(buffer == data->Buffers.at(bufferIdx % SFX::StreamState::BUFFER_COUNT), "Unqueued buffer is not the same as expected!");
							}
							else if (error != AL_INVALID_VALUE)
							{
								ZE_CODE_ERROR(ZE_AL_ERROR(error), "Error unqueuing buffer from stream!");
								++data->OrderData.NextSubmitBufferIdx;
								co_return endSubmission();
							}

							alSourceQueueBuffers(data->Source, 1, &buffer);
							error = alGetError();
							if (error != AL_NO_ERROR)
							{
								ZE_CODE_ERROR(ZE_AL_ERROR(error), "Error queuing buffer to stream!");
							}

							// Indicate for others that stream has ended
							if (!data->EnableLoop && data->StateInfo.IsStreamEnd())
								data->OrderData.Alive = false;
						}
						const U64 nextTask = ++data->OrderData.NextSubmitBufferIdx;
						endSubmission();

						// Last check if should ignore everything and exit
						if (!data->OrderData.Alive)
							co_return;

						co_return data->OrderData.PerformNextWaitingTask(nextTask);
					}, it->second, it->second->NextLoadBufferIdx++);
			}
		}
	}
}
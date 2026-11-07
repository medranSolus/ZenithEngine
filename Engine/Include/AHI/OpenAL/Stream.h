#pragma once
#include "SFX/StreamDesc.h"
#include "SFX/StreamOrderAwaiter.h"

namespace ZE::AHI::OpenAL
{
	class Stream final
	{
		struct StreamData
		{
			std::array<Move<ALuint, UINT_MAX>, SFX::StreamState::BUFFER_COUNT> Buffers;
			std::unique_ptr<U8[]> RingBuffer;
			std::shared_ptr<IO::File> FileHandle;
			SFX::StreamState StateInfo = {};
			Move<ALuint, UINT_MAX> Source;
			bool EnableLoop = false;

			UA64 NextLoadBufferIdx = 0;
			UA8 BuffersInFlight = 0;
			SFX::StreamOrderData OrderData = {};

			~StreamData();
		};

		static inline std::unordered_map<ALuint, std::shared_ptr<StreamData>> activeStreams;

		std::shared_ptr<StreamData> data;
		Move<ALuint, UINT_MAX> source;

	public:
		Stream() = default;
		ZE_CLASS_MOVE(Stream);
		~Stream();

		static Expected<Stream> Create(SFX::Device& dev, const SFX::StreamDesc& desc) noexcept;

		Status SetVolume(float decibels) noexcept { alSourcef(source, AL_GAIN, GetGainLevel(decibels));  ZE_AL_RET_FAILED(); return {}; }
		Status SetPitch(float semitones) noexcept { alSourcef(source, AL_PITCH, GetPitchLevel(semitones)); ZE_AL_RET_FAILED(); return {}; }

		Status Resume() noexcept { return ResumePlayback(source); }
		Status Pause() noexcept { alSourcePause(source); ZE_AL_RET_FAILED(); return {}; }

		bool IsLooping() const noexcept { ZE_ASSERT(data, "Not initialized!"); return data->EnableLoop; }

		Task<Status> ResetPosition(bool autoPlay, bool enableLoop) noexcept;

		// Audio API internal

		static void OnBufferEndCallback(ALuint source, U8 buffersFinished) noexcept;
	};
}
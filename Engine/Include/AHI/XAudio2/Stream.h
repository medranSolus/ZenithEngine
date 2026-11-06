#pragma once
#include "SFX/StreamDesc.h"
#include "SFX/StreamOrderAwaiter.h"

namespace ZE::AHI::XAudio2
{
	class Stream final
	{
		struct StreamData
		{
			std::unique_ptr<U8[]> RingBuffer;
			std::shared_ptr<IO::File> FileHandle;
			SFX::StreamState StateInfo = {};
			Ptr<IXAudio2SourceVoice> SourceVoice;
			bool EnableLoop = false;
			SFX::StreamOrderData OrderData = {};

			~StreamData();
		};

		class StreamCallback final : public IXAudio2VoiceCallback
		{
			std::shared_ptr<StreamData> data;

		public:
			StreamCallback(std::shared_ptr<StreamData> data) noexcept : data(data) {}
			ZE_CLASS_DEFAULT(StreamCallback);
			virtual ~StreamCallback() = default;

			void OnVoiceProcessingPassStart(U32 bytesRequired) noexcept override {}
			void OnVoiceProcessingPassEnd() noexcept override {}
			void OnBufferStart(void* bufferCtx) noexcept override {}
			void OnLoopEnd(void* bufferCtx) noexcept override {}

			void OnStreamEnd() noexcept override { data->OrderData.Alive = false; }
			void OnVoiceError(void* bufferCtx, HRESULT error) noexcept override { ZE_CODE_ERROR(ZE_XA2_ERROR(error), "Error while playing stream number " + std::to_string(reinterpret_cast<U64>(bufferCtx))); }

			void OnBufferEnd(void* bufferCtx) noexcept override;
		};

		std::shared_ptr<StreamData> data;
		std::unique_ptr<StreamCallback> callback;

		Status SubmitInitialBuffers() noexcept;

	public:
		Stream() = default;
		ZE_CLASS_MOVE(Stream);
		~Stream();

		static Expected<Stream> Create(SFX::Device& dev, const SFX::StreamDesc& desc) noexcept;

		Status SetVolume(float decibels) noexcept { ZE_ASSERT(data, "Not initialized!"); return ZE_XA2_ERROR(data->SourceVoice->SetVolume(GetVolumeLevel(decibels), XAUDIO2_COMMIT_NOW)); }
		Status SetPitch(float semitones) noexcept { ZE_ASSERT(data, "Not initialized!"); return ZE_XA2_ERROR(data->SourceVoice->SetFrequencyRatio(GetPitchLevel(semitones), XAUDIO2_COMMIT_NOW)); }

		Status Resume() noexcept { ZE_ASSERT(data, "Not initialized!"); return ZE_XA2_ERROR(data->SourceVoice->Start(0, XAUDIO2_COMMIT_NOW)); }
		Status Pause() noexcept { ZE_ASSERT(data, "Not initialized!"); return ZE_XA2_ERROR(data->SourceVoice->Stop(0, XAUDIO2_COMMIT_NOW)); }

		bool IsLooping() const noexcept { ZE_ASSERT(data, "Not initialized!"); return data->EnableLoop; }

		Task<Status> ResetPosition(bool autoPlay, bool enableLoop) noexcept;
	};
}
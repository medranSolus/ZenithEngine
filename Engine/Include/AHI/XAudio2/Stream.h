#pragma once
#include "SFX/StreamDesc.h"

namespace ZE::AHI::XAudio2
{
	class Stream final
	{
		struct StreamData
		{
			struct Awaiter
			{
				Ptr<StreamData> Data;
				U64 BufferIdx;
				std::coroutine_handle<> Handle = {};

				bool await_ready() const noexcept;
				bool await_suspend(std::coroutine_handle<> h) noexcept;
				bool await_resume() const noexcept;
			};

			std::unique_ptr<U8[]> RingBuffer;
			std::shared_ptr<IO::File> FileHandle;
			SFX::StreamState StateInfo = {};
			Ptr<IXAudio2SourceVoice> SourceVoice;

			UA64 NextSubmitBufferIdx = 0;
			UA64 NextLoadedBufferIdx = 0;
			BoolAtom Alive = true;
			std::array<std::coroutine_handle<>, SFX::StreamState::BUFFER_COUNT> WaitTable;
			SpinLock WaitLock;

			~StreamData();

			void AbortAllWaiters() noexcept;
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
			void OnStreamEnd() noexcept override {}
			void OnBufferStart(void* bufferCtx) noexcept override {}
			void OnLoopEnd(void* bufferCtx) noexcept override {}
			void OnVoiceError(void* bufferCtx, HRESULT error) noexcept override {}

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

		void SetVolume(float decibels) noexcept { data->SourceVoice->SetVolume(GetVolumeLevel(decibels), XAUDIO2_COMMIT_NOW); }
		void SetPitch(float semitones) noexcept { data->SourceVoice->SetFrequencyRatio(GetPitchLevel(semitones), XAUDIO2_COMMIT_NOW); }

		void Resume() noexcept { data->SourceVoice->Start(0, XAUDIO2_COMMIT_NOW); }
		void Pause() noexcept { data->SourceVoice->Stop(0, XAUDIO2_COMMIT_NOW); }

		Task<Status> ResetPosition(bool autoPlay) noexcept;
	};
}
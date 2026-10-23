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
			bool EnableLoop = false;
			SFX::StreamState StateInfo = {};
			Ptr<IXAudio2SourceVoice> SourceVoice;

			UA64 NextSubmitBufferIdx = 0;
			BoolAtom Alive = true;
			UA8 FinishedBuffers = 0;
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

		void SetVolume(float decibels) noexcept { ZE_ASSERT(data, "Not initialized!"); data->SourceVoice->SetVolume(GetVolumeLevel(decibels), XAUDIO2_COMMIT_NOW); }
		void SetPitch(float semitones) noexcept { ZE_ASSERT(data, "Not initialized!"); data->SourceVoice->SetFrequencyRatio(GetPitchLevel(semitones), XAUDIO2_COMMIT_NOW); }

		void Resume() noexcept { ZE_ASSERT(data, "Not initialized!"); data->SourceVoice->Start(0, XAUDIO2_COMMIT_NOW); }
		void Pause() noexcept { ZE_ASSERT(data, "Not initialized!"); data->SourceVoice->Stop(0, XAUDIO2_COMMIT_NOW); }

		bool IsLooping() const noexcept { ZE_ASSERT(data, "Not initialized!"); return data->EnableLoop; }

		Task<Status> ResetPosition(bool autoPlay, bool enableLoop) noexcept;
	};
}
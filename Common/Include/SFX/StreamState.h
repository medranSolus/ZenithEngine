#pragma once
#include "AudioBuffer.h"
#include "FileUtils.h"

namespace ZE::SFX
{
	// Current state of the streamed audio file depending on file source type
	class StreamState final
	{
	public:
		static constexpr U8 BUFFER_COUNT = 4;
		static constexpr U64 MAX_BUFFER_SIZE = 32 * Math::KILOBYTE;

	private:
		struct StateWAV
		{
			Ptr<IO::File> File;
			U64 DataStart;
			U64 CurrentPosition;
			U64 FileEnd;
		};

		// Usable part of the MAX_BUFFER_SIZE that ring buffer should be allocated with
		U32 audioBlockSize = 0;
		FileSourceType sourceType = FileSourceType::Unknown;
		union
		{
			StateWAV wav;
			FileUtils::FlacCtx flac;
			FileUtils::OggVorbisCtx oggVorbis;
			FileUtils::OggOpusCtx oggOpus;
		};

		void MoveFrom(StreamState&& state) noexcept;
		void Release() noexcept;

	public:
		constexpr StreamState() noexcept {}
		ZE_CLASS_NO_COPY(StreamState);
		StreamState(StreamState&& state) noexcept { MoveFrom(std::move(state)); }
		StreamState& operator=(StreamState&& state) noexcept { Release(); MoveFrom(std::move(state)); return *this; }
		~StreamState();

		// Get audio file description from any file region on disk for async streaming
		static Expected<StreamState> Create(IO::File& file, U64 startOffset, U64 regionSize, FileSourceType type, AudioDesc& audioInfo) noexcept;

		constexpr U32 GetBlockSize() const noexcept { return audioBlockSize; }

		// Asynchronously load next chunk in stream to the memory (can return EOF if looping is not enabled)
		Task<Status> ReadNextChunk(U32 readSize, U8* destBuffer, bool loopAudio) noexcept;
		// Move current stream position to the begining of the track
		Status RewindToBegining(bool flushPendingAudio) noexcept;
		// Check if all samples data has been loaded already
		bool IsStreamEnd() const noexcept;
	};

}
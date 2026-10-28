#include "SFX/StreamState.h"
#include "SFX/FileError.h"
#include "IO/WAV/Utils.h"

namespace ZE::SFX
{
	void StreamState::MoveFrom(StreamState&& state) noexcept
	{
		audioBlockSize = state.audioBlockSize;
		sourceType = state.sourceType;
		switch (sourceType)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FileSourceType::Unknown:
			break;
		case FileSourceType::WAV:
		{
			wav = std::move(state.wav);
			break;
		}
		case FileSourceType::Flac:
		{
			flac = std::move(state.flac);
			break;
		}
		case FileSourceType::OggVorbis:
		{
			oggVorbis = std::move(state.oggVorbis);
			break;
		}
		case FileSourceType::OggOpus:
		{
			oggOpus = std::move(state.oggOpus);
			break;
		}
		}
	}

	void StreamState::Release() noexcept
	{
		switch (sourceType)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FileSourceType::Unknown:
		case FileSourceType::WAV:
			break;
		case FileSourceType::Flac:
		{
			if (flac.Decoder)
			{
				FLAC__stream_decoder_finish(flac.Decoder);
				FLAC__stream_decoder_delete(flac.Decoder);
				flac.Decoder = nullptr;
			}
			break;
		}
		case FileSourceType::OggVorbis:
		{
			if (oggOpus.Decoder)
			{
				ov_clear(oggVorbis.Decoder);
				oggVorbis.Decoder = nullptr;
			}
			break;
		}
		case FileSourceType::OggOpus:
		{
			if (oggOpus.Decoder)
			{
				op_free(oggOpus.Decoder);
				oggOpus.Decoder = nullptr;
			}
			break;
		}
		}
	}

	StreamState::~StreamState()
	{
		switch (sourceType)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FileSourceType::Unknown:
			break;
		case FileSourceType::WAV:
		{
			wav.~StateWAV();
			break;
		}
		case FileSourceType::Flac:
		{
			flac.~FlacCtx();
			break;
		}
		case FileSourceType::OggVorbis:
		{
			oggVorbis.~OggVorbisCtx();
			break;
		}
		case FileSourceType::OggOpus:
		{
			oggOpus.~OggOpusCtx();
			break;
		}
		}
	}

	Expected<StreamState> StreamState::Create(IO::File& file, U64 startOffset, U64 regionSize, FileSourceType type, AudioDesc& audioInfo) noexcept
	{
		StreamState state = {};
		state.sourceType = type;
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FileSourceType::Unknown:
		{
			ZE_FAIL("Unknown audio file format!");
			return std::unexpected(IO::WAV::Error::Make(IO::WAV::FileResult::Unknown));
		}
		case FileSourceType::WAV:
		{
			new (&state.wav)StreamState::StateWAV;
			ZE_EXPECT_RET_FAILED(audioInfo, IO::WAV::ParseFileInfo(file, state.wav.DataStart, startOffset));
			state.wav.File = &file;
			state.wav.FileEnd = startOffset + regionSize;
			break;
		}
		case FileSourceType::Flac:
		{
			new (&state.flac)FileUtils::FlacCtx;
			ZE_EXPECT_RET_FAILED(state.flac, FileUtils::GetFlacCtx(file, startOffset, regionSize, audioInfo));
			break;
		}
		case FileSourceType::OggVorbis:
		{
			new (&state.oggVorbis)FileUtils::OggVorbisCtx;
			ZE_EXPECT_RET_FAILED(state.oggVorbis, FileUtils::GetOggVorbisCtx(file, startOffset + regionSize, audioInfo));
			break;
		}
		case FileSourceType::OggOpus:
		{
			new (&state.oggOpus)FileUtils::OggOpusCtx;
			ZE_EXPECT_RET_FAILED(state.oggOpus, FileUtils::GetOggOpusCtx(file, startOffset + regionSize, audioInfo));
			break;
		}
		}
		state.audioBlockSize = Math::DivideRoundUp<U8>(audioInfo.BitsPerSample, 8) * Intrin::CountBitsSet(static_cast<U32>(audioInfo.Channels));
		state.audioBlockSize = (StreamState::MAX_BUFFER_SIZE / state.audioBlockSize) * state.audioBlockSize;
		return state;
	}

	Task<Status> StreamState::ReadNextChunk(U32 readSize, U8* destBuffer, bool loopAudio) noexcept
	{
		ZE_ASSERT(destBuffer, "Empty output buffer!");

		switch (sourceType)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FileSourceType::Unknown:
		{
			ZE_FAIL("Unknown audio file format!");
			co_return IO::WAV::Error::Make(IO::WAV::FileResult::Unknown);
		}
		case FileSourceType::WAV:
		{
			ZE_ASSERT(wav.File, "WAV ctx not initialized!");
			Status stat = co_await wav.File->ReadAsync(destBuffer, readSize, wav.DataStart + wav.CurrentPosition);
			if (stat)
			{
				if (IO::EofResult::IsEOF(stat))
				{
					U32 bytesRead = IO::EofResult::GetRealBytes(stat);
					if (loopAudio)
					{
						readSize -= bytesRead;
						stat = co_await wav.File->ReadAsync(destBuffer + bytesRead, readSize, wav.DataStart);
						if (stat)
							co_return stat;
						wav.CurrentPosition = wav.DataStart + readSize;
					}
					else
					{
						wav.CurrentPosition += bytesRead;
						co_return stat;
					}
				}
				else
					co_return stat;
			}
			else
				wav.CurrentPosition += readSize;
			break;
		}
		case FileSourceType::Flac:
		{
			ZE_ASSERT(flac.Decoder, "Flac ctx not initialized!");

			flac.IoCtx->OutBuffer = destBuffer;
			flac.IoCtx->BufferSize = readSize;
			flac.IoCtx->WriteOffset = 0;
			do
			{
				if (!FLAC__stream_decoder_process_single(flac.Decoder))
					co_return flac.IoCtx->Code;

				if (FLAC__stream_decoder_get_state(flac.Decoder) == FLAC__STREAM_DECODER_END_OF_STREAM)
				{
					if (loopAudio)
					{
						ZE_CODE_RET_FAILED_CORO(RewindToBegining(false));
					}
					else
						co_return IO::EofResult::Make(flac.IoCtx->WriteOffset);
				}
				else if (flac.IoCtx->RolloverBuffer.size()) // Means we already filled entire buffer
					break;
			} while (flac.IoCtx->WriteOffset < readSize);
			break;
		}
		case FileSourceType::OggVorbis:
		{
			ZE_ASSERT(oggVorbis.Decoder, "Ogg vorbis ctx not initialized!");

			U32 bytesRead = 0;
			ZE_EXPECT_RET_FAILED_CODE_CORO(bytesRead, FileUtils::ReadOggVorbis(oggVorbis,
				readSize, reinterpret_cast<float*>(destBuffer)));
			if (bytesRead < readSize)
			{
				if (loopAudio)
				{
					ZE_CODE_RET_FAILED_CORO(RewindToBegining(false));

					readSize -= bytesRead;
					ZE_EXPECT_RET_FAILED_CODE_CORO(bytesRead, FileUtils::ReadOggVorbis(oggVorbis,
						readSize, reinterpret_cast<float*>(destBuffer + bytesRead)));
					if (bytesRead != readSize)
						co_return IO::EofResult::Make(bytesRead);
				}
				else
					co_return IO::EofResult::Make(bytesRead);
			}
			break;
		}
		case FileSourceType::OggOpus:
		{
			ZE_ASSERT(oggOpus.Decoder, "Ogg opus ctx not initialized!");

			U32 bytesRead = 0;
			ZE_EXPECT_RET_FAILED_CODE_CORO(bytesRead, FileUtils::ReadOggOpus(oggOpus,
				readSize, reinterpret_cast<float*>(destBuffer)));
			if (bytesRead < readSize)
			{
				if (loopAudio)
				{
					ZE_CODE_RET_FAILED_CORO(RewindToBegining(false));

					readSize -= bytesRead;
					ZE_EXPECT_RET_FAILED_CODE_CORO(bytesRead, FileUtils::ReadOggOpus(oggOpus,
						readSize, reinterpret_cast<float*>(destBuffer + bytesRead)));
					if (bytesRead != readSize)
						co_return IO::EofResult::Make(bytesRead);
				}
				else
					co_return IO::EofResult::Make(bytesRead);
			}
			break;
		}
		}
		co_return Status{};
	}

	Status StreamState::RewindToBegining(bool flushPendingAudio) noexcept
	{
		switch (sourceType)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FileSourceType::Unknown:
		{
			ZE_FAIL("Unknown audio file format!");
			return IO::WAV::Error::Make(IO::WAV::FileResult::Unknown);
		}
		case FileSourceType::WAV:
		{
			ZE_ASSERT(wav.File, "WAV ctx not initialized!");
			wav.CurrentPosition = wav.DataStart;
			break;
		}
		case FileSourceType::Flac:
		{
			ZE_ASSERT(flac.Decoder, "Flac ctx not initialized!");
			if (!FLAC__stream_decoder_seek_absolute(flac.Decoder, 0))
			{
				if (FLAC__stream_decoder_get_state(flac.Decoder) == FLAC__STREAM_DECODER_SEEK_ERROR)
					FLAC__stream_decoder_flush(flac.Decoder);
				return flac.IoCtx->Code;
			}
			if (flushPendingAudio)
				flac.IoCtx->RolloverBuffer.clear();
			break;
		}
		case FileSourceType::OggVorbis:
		{
			ZE_ASSERT(oggVorbis.Decoder, "Ogg vorbis ctx not initialized!");
			return ZE_VORBIS_ERROR(ov_pcm_seek(oggVorbis.Decoder, 0));
		}
		case FileSourceType::OggOpus:
		{
			ZE_ASSERT(oggOpus.Decoder, "Ogg opus ctx not initialized!");
			return ZE_OPUS_ERROR(op_pcm_seek(oggOpus.Decoder, 0));
		}
		}
		return {};
	}

	bool StreamState::IsStreamEnd() const noexcept
	{
		switch (sourceType)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FileSourceType::Unknown:
			return true;
		case FileSourceType::WAV:
		{
			ZE_ASSERT(wav.File, "WAV ctx not initialized!");
			return wav.CurrentPosition >= wav.FileEnd;
		}
		case FileSourceType::Flac:
		{
			ZE_ASSERT(flac.Decoder, "Flac ctx not initialized!");
			return flac.IoCtx->ReadOffset >= flac.IoCtx->FileEnd;
		}
		case FileSourceType::OggVorbis:
		{
			ZE_ASSERT(oggVorbis.Decoder, "Ogg vorbis ctx not initialized!");
			return oggVorbis.IoCtx->ReadOffset >= oggVorbis.IoCtx->FileEnd;
		}
		case FileSourceType::OggOpus:
		{
			ZE_ASSERT(oggOpus.Decoder, "Ogg opus ctx not initialized!");
			return oggOpus.IoCtx->ReadOffset >= oggVorbis.IoCtx->FileEnd;
		}
		}
	}
}
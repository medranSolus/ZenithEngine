#include "SFX/AudioBuffer.h"
#include "SFX/FileError.h"
#include "IO/WAV/Utils.h"
ZE_WARNING_PUSH
#include "ogg/ogg.h"
#include "vorbis/vorbisfile.h"
#include "opus.h"
ZE_WARNING_POP

namespace ZE::SFX
{
	Expected<AudioBuffer> LoadFile(std::string_view filename) noexcept
	{
		const std::filesystem::path path(filename);
		std::string ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](char c) { return static_cast<char>(std::tolower(c)); });

		IO::File file;
		Status errorCode = file.Open(filename, Base(IO::FileFlag::DefaultRead));
		if (errorCode)
		{
			ZE_CODE_ERROR(errorCode, "Error openinig \"" + path.string() + "\" file!");
			return std::unexpected(errorCode);
		}

		if (ext == ".wav")
		{
			auto buffer = IO::WAV::ParseFileInfo(file);
			if (!buffer)
			{
				ZE_CODE_ERROR(buffer.error(), "Error parsing header of \"" + path.string() + "\" file!");
				return std::unexpected(buffer.error());
			}
			errorCode = IO::WAV::LoadSampleData(file, *buffer, 0, 0);
			if (errorCode)
			{
				ZE_CODE_ERROR(errorCode, "Error loading sample data of \"" + path.string() + "\" file!");
				return std::unexpected(errorCode);
			}
			return *buffer;
		}
		else if (ext == ".flac")
		{
			FLAC__StreamDecoder* decoder = FLAC__stream_decoder_new();
			if (!decoder)
			{
				ZE_FAIL("Failed to create FLAC decoder!");
				return std::unexpected(ZE_FLAC_DECODER_INIT_ERROR(FLAC__STREAM_DECODER_INIT_STATUS_MEMORY_ALLOCATION_ERROR)	);
			}
			FLAC__stream_decoder_set_md5_checking(decoder, _ZE_MODE_DEBUG);
			FLAC__stream_decoder_set_metadata_respond(decoder, FLAC__METADATA_TYPE_STREAMINFO);
			FLAC__stream_decoder_set_metadata_respond(decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);

			struct FlacCtx
			{
				IO::File& File;
				Status& Code;
				SFX::AudioBuffer Buffer = {};
				U8 ChannelCount = 0;
				U32 WriteOffset = 0;
			};

			FlacCtx ctx = { file, errorCode };
			FLAC__StreamDecoderReadCallback read = [](const FLAC__StreamDecoder* decoder, FLAC__byte buffer[], size_t* bytes, void* ctx) noexcept -> FLAC__StreamDecoderReadStatus
				{
					ZE_ASSERT(ctx, "Empty FLAC context!");
					ZE_ASSERT(bytes, "Empty FLAC byte count!");
					ZE_ASSERT(buffer, "Empty FLAC output buffer!");

					if (*bytes > 0)
					{
						FILE* file = reinterpret_cast<FlacCtx*>(ctx)->File.GetHandle();
						*bytes = std::fread(buffer, sizeof(FLAC__byte), *bytes, file);

						if (std::ferror(file))
							return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
						if (*bytes == 0)
							return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
						return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
					}
					else
						return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
				};
			FLAC__StreamDecoderSeekCallback seek = [](const FLAC__StreamDecoder* decoder, FLAC__uint64 offset, void* ctx) noexcept -> FLAC__StreamDecoderSeekStatus
				{
					ZE_ASSERT(ctx, "Empty FLAC context!");
					reinterpret_cast<FlacCtx*>(ctx)->File.SetOffset(offset);
					return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
				};
			FLAC__StreamDecoderTellCallback tell = [](const FLAC__StreamDecoder* decoder, FLAC__uint64* offset, void* ctx) noexcept -> FLAC__StreamDecoderTellStatus
				{
					ZE_ASSERT(ctx, "Empty FLAC context!");
					ZE_ASSERT(offset, "Empty FLAC offset!");

					*offset = reinterpret_cast<FlacCtx*>(ctx)->File.GetOffset();
					return FLAC__STREAM_DECODER_TELL_STATUS_OK;
				};
			FLAC__StreamDecoderLengthCallback length = [](const FLAC__StreamDecoder* decoder, FLAC__uint64* streamLen, void* ctx) noexcept -> FLAC__StreamDecoderLengthStatus
				{
					ZE_ASSERT(ctx, "Empty FLAC context!");
					ZE_ASSERT(streamLen, "Empty stream length!");

					auto& context = *reinterpret_cast<FlacCtx*>(ctx);
					auto size = context.File.GetSize();
					if (size)
					{
						*streamLen = *size;
						return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
					}
					context.Code = size.error();
					return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
				};
			FLAC__StreamDecoderEofCallback eof = [](const FLAC__StreamDecoder* decoder, void* ctx) noexcept -> FLAC__bool
				{
					ZE_ASSERT(ctx, "Empty FLAC context!");
					return std::feof(reinterpret_cast<FlacCtx*>(ctx)->File.GetHandle());
				};
			FLAC__StreamDecoderWriteCallback write = [](const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* ctx) noexcept -> FLAC__StreamDecoderWriteStatus
				{
					ZE_ASSERT(frame, "Empty FLAC frame!");
					ZE_ASSERT(buffer, "Empty FLAC buffer!");
					ZE_ASSERT(ctx, "Empty FLAC context!");

					auto& context = *reinterpret_cast<FlacCtx*>(ctx);

					// Sanity check
					const U8 sampleBytes = Math::DivideRoundUp<U8>(context.Buffer.BitsPerSample, 8);
					if (context.Buffer.Bytes < context.WriteOffset + (frame->header.blocksize * context.ChannelCount * sampleBytes))
						return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

					// Copy into interleaved format
					for (U32 i = 0; i < frame->header.blocksize; ++i)
					{
						for (U8 j = 0; j < context.ChannelCount; ++j)
						{
							std::memcpy(context.Buffer.Samples.get() + context.WriteOffset, buffer[j] + i, sampleBytes);
							context.WriteOffset += sampleBytes;
						}
					}
					return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
				};
			FLAC__StreamDecoderMetadataCallback metadata = [](const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata, void* ctx) noexcept -> void
				{
					ZE_ASSERT(metadata, "Empty FLAC metadata!");
					ZE_ASSERT(ctx, "Empty FLAC context!");

					switch (metadata->type)
					{
					case FLAC__METADATA_TYPE_STREAMINFO:
					{
						auto& context = *reinterpret_cast<FlacCtx*>(ctx);
						ZE_ASSERT(context.Buffer.Samples == nullptr, "FLAC STREAMINFO metadata already processed!");

						context.ChannelCount = Utils::SafeCast<U8>(metadata->data.stream_info.channels);
						context.Buffer.Bytes = Utils::SafeCast<U32>(metadata->data.stream_info.total_samples * context.ChannelCount * Math::DivideRoundUp(metadata->data.stream_info.bits_per_sample, 8U));
						context.Buffer.SampleRate = metadata->data.stream_info.sample_rate;
						context.Buffer.BitsPerSample = Utils::SafeCast<U8>(metadata->data.stream_info.bits_per_sample);
						context.Buffer.IsFloat = false;
						context.Buffer.Samples = std::make_shared<U8[]>(context.Buffer.Bytes);
						break;
					}
					case FLAC__METADATA_TYPE_VORBIS_COMMENT:
					{
						constexpr const char* CHANNELMASK_TAG = "channelmask=";
						for (U32 i = 0; i < metadata->data.vorbis_comment.num_comments; ++i)
						{
							auto& comment = metadata->data.vorbis_comment.comments[i];
							bool found = true;
							for (U32 j = 0; j < comment.length && j < 12; ++j)
							{
								if (std::tolower(comment.entry[j]) != CHANNELMASK_TAG[j])
								{
									found = false;
									break;
								}
							}
							// Parse channel mask
							if (found)
							{
								// Same as values used in the specification
								reinterpret_cast<FlacCtx*>(ctx)->Buffer.Channels = std::strtoul(reinterpret_cast<const char*>(comment.entry + 12), nullptr, 0);
								break;
							}
						}
						break;
					}
					default:
					{
						if (metadata->is_last)
						{
							auto& context = *reinterpret_cast<FlacCtx*>(ctx);

							if (context.Buffer.Samples == nullptr)
							{
								ZE_FAIL("FLAC STREAMINFO metadata has not been processed!");
								context.Code = ZE_FLAC_DECODER_ERROR(FLAC__STREAM_DECODER_ERROR_STATUS_BAD_METADATA);
							}
						}
						break;
					}
					}
				};
			FLAC__StreamDecoderErrorCallback error = [](const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* ctx) noexcept -> void
				{
					ZE_ASSERT(ctx, "Empty FLAC context!");
					// Just save the error for retrieval later
					reinterpret_cast<FlacCtx*>(ctx)->Code = ZE_FLAC_DECODER_ERROR(status);
				};

			ZE_WARNING_DISABLE_MSVC(5039);
			FLAC__StreamDecoderInitStatus initStatus = FLAC__stream_decoder_init_stream(decoder, read, seek, tell, length, eof, write, metadata, error, &ctx);
			if (initStatus == FLAC__STREAM_DECODER_INIT_STATUS_OK)
			{
				FLAC__stream_decoder_process_until_end_of_metadata(decoder);

				if (ctx.Buffer.Channels == 0)
				{
					// Fallback when no channel mask is provided
					ctx.Buffer.Channels = GetDefaultMask(ctx.ChannelCount);
					if (ctx.Buffer.Channels == 0)
						ctx.Code = ZE_FLAC_DECODER_ERROR(FLAC__STREAM_DECODER_ERROR_STATUS_BAD_METADATA);
				}

				if (!ctx.Code)
					FLAC__stream_decoder_process_until_end_of_stream(decoder);
				FLAC__stream_decoder_finish(decoder);
			}
			else
				errorCode = ZE_FLAC_DECODER_INIT_ERROR(initStatus);

			FLAC__stream_decoder_delete(decoder);
			if (!errorCode)
				return ctx.Buffer;
		}
		else if (ext == ".ogg")
		{
		}
		else if (ext == ".opus")
		{
		}
		else
		{
			ZE_FAIL("Unsupported audio file format: \"" + ext + "\"");
			return std::unexpected(IO::WAV::Error::Make(IO::WAV::FileResult::Unknown));
		}
		return std::unexpected(errorCode);
	}
}
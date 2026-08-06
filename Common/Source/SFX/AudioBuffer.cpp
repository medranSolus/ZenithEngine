#include "SFX/AudioBuffer.h"
#include "IO/WAV/Utils.h"
ZE_WARNING_PUSH
#include "FLAC/stream_decoder.h"
#include "ogg/ogg.h"
#include "vorbis/vorbisfile.h"
#include "opus.h"
ZE_WARNING_POP

namespace ZE::SFX
{
	Status AudioBuffer::LoadFile(std::string_view filename) noexcept
	{
		const std::filesystem::path path(filename);
		std::string ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](char c) { return static_cast<char>(std::tolower(c)); });

		IO::File file;
		if (Status code = file.Open(filename, Base(IO::FileFlag::DefaultRead)))
		{
			ZE_CODE_ERROR(code, "Error openinig \"" + path.string() + "\" file!");
			return code;
		}

		if (ext == ".wav")
		{
			auto buffer = IO::WAV::ParseFileInfo(file);
			if (!buffer)
			{
				ZE_CODE_ERROR(buffer.error(), "Error parsing header of \"" + path.string() + "\" file!");
				return buffer.error();
			}
			if (Status code = IO::WAV::LoadSampleData(file, *buffer, 0, 0))
			{
				ZE_CODE_ERROR(code, "Error loading sample data of \"" + path.string() + "\" file!");
				return code;
			}
			*this = std::move(*buffer);
		}
		else if (ext == ".flac")
		{
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
			return IO::WAV::Error::Make(IO::WAV::FileResult::Unknown);
		}
		return {};
	}
}
#include "SFX/AudioBank.h"
#include "Settings.h"

namespace ZE::SFX
{
	Task<Expected<FxID>> AudioBank::LoadFileAsync(std::string_view filename) noexcept
	{
		return Settings::GetThreadPool().Schedule(ThreadPriority::Normal, [filename, this]() -> Expected<FxID> { return LoadFile(filename); });
	}

	Expected<FxID> AudioBank::LoadFile(std::string_view filename) noexcept
	{
		IO::File file;
		Status errorCode = file.Open(filename, Base(IO::FileFlag::DefaultRead));
		if (errorCode)
		{
			ZE_CODE_ERROR(errorCode, "Error openinig \"" + std::string(filename) + "\" file!");
			return std::unexpected(errorCode);
		}

		std::string ext = std::filesystem::path(filename).extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](char c) { return static_cast<char>(std::tolower(c)); });

		FileSourceType type = FileSourceType::Unknown;
		if (ext == ".wav")
			type = FileSourceType::WAV;
		else if (ext == ".flac")
			type = FileSourceType::Flac;
		else if (ext == ".ogg")
			type = FileSourceType::OggVorbis;
		else if (ext == ".opus")
			type = FileSourceType::OggOpus;
		else
		{
			ZE_FAIL("Unknown audio file type!");
			return std::unexpected(IO::WAV::Error::Make(IO::WAV::FileResult::Unknown));
		}

		U64 fileSize = 0;
		ZE_EXPECT_RET_FAILED(fileSize, file.GetSize());
		AudioBuffer buffer = {};
		ZE_EXPECT_RET_FAILED(buffer, SFX::LoadFile(file, 0, fileSize, type));

		LockGuardRW lock(loadLock);

		FxID id = soundRegistry.create();
		soundRegistry.emplace<AudioBuffer>(id, std::move(buffer));
#if _ZE_MODE_DEBUG
		soundRegistry.emplace<std::string>(id, filename);
#endif
		return id;
	}
}
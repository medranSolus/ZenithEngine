#pragma once
#include "SoundGroup.h"

namespace ZE::SFX
{
	// Information for the Stream creation
	struct StreamDesc
	{
		std::shared_ptr<IO::File> FileHandle;
		U64 FileStart = 0;
		U64 RegionSize = 0;
		FileSourceType SourceType = FileSourceType::Unknown;
		bool LoopAudio = false;
		const SoundGroup* Group = nullptr;
	};
}
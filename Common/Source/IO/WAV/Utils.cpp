#include "IO/WAV/Utils.h"

namespace ZE::IO::WAV
{
	Status LoadSampleData(File& file, U8* sampleBuffer, U32 blockSize, U32 writeOffset) noexcept
	{
		ZE_ASSERT(sampleBuffer && blockSize, "Buffer must be allocated before loading sample data!");
		ZE_ASSERT(blockSize + writeOffset <= blockSize, "Reading beyond buffer bounds!");

		ZE_CODE_RET_FAILED(file.Read(sampleBuffer + writeOffset, blockSize));
		return {};
	}
}
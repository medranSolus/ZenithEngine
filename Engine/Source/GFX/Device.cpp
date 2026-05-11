#include "GFX/Device.h"

namespace ZE::GFX
{
	Status Device::FlushGPU() noexcept
	{
		U64 val = 0;
		ZE_EXPECT_RET_FAILED_CODE(val, SetMainFence());
		ZE_CODE_RET_FAILED(WaitMain(val));
		ZE_EXPECT_RET_FAILED_CODE(val, SetComputeFence());
		ZE_CODE_RET_FAILED(WaitCompute(val));
		ZE_EXPECT_RET_FAILED_CODE(val, SetCopyFence());
		ZE_CODE_RET_FAILED(WaitCopy(val));
		return {};
	}
}
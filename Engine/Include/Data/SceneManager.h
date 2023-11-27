#pragma once
#include "AssetsStreamer.h"

namespace ZE::Data
{
#if _ZE_EXTERNAL_MODEL_LOADING
	// Load model data from external source
	Task<bool> LoadExternalModel(GFX::Device& dev, AssetsStreamer& assets, EID root, std::string_view filename) noexcept;
#endif
}
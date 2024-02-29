#pragma once
#include "AssetsStreamer.h"
#include "Transform.h"

namespace ZE::Data
{
#if _ZE_EXTERNAL_MODEL_LOADING
	// Load model data from external source
	Task<bool> LoadExternalModel(GFX::Device& dev, AssetsStreamer& assets, EID root, const Data::Transform& transform,
		std::string_view filename, ExternalModelOptions options = static_cast<ExternalModelOptions>(ExternalModelOption::None)) noexcept;
#endif
}
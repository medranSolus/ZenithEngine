#pragma once
#include "AssetsStreamer.h"
#include "Transform.h"

namespace ZE::Data
{
	// ID for mesh that is currently being loaded into some foreign registry
	struct LoadingMeshID { EID ID = INVALID_EID; };
	// ID for material that is currently being loaded into some foreign registry
	struct LoadingMaterialID { EID ID = INVALID_EID; };

#if _ZE_EXTERNAL_MODEL_LOADING
	// Load model data from external source
	Task<Status> LoadExternalModel(GFX::Device& dev, AssetsStreamer& assets, EID root, const Data::Transform& transform,
		std::string_view filename, ExternalModelOptions options = Base(ExternalModelOption::None)) noexcept;
#endif
}
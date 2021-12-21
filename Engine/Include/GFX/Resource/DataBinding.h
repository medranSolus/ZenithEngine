#pragma once
#include "GFX/API/DX11/Resource/DataBinding.h"
#include "GFX/API/DX12/Resource/DataBinding.h"

namespace ZE::GFX::Resource
{
	// Resource bindings used in shaders
	class DataBinding final
	{
		// There should be 'renderer' bindings, that are appended to the signature,
		// and also there should be a way to just call DataBinding.Bind(Material) to set all the needed stuff.
		// But at the same time we should also set all the renderer stuff, so we need to access renderer data too...
		ZE_API_BACKEND(Resource::DataBinding);

	public:
		DataBinding() = default;
		ZE_CLASS_MOVE(DataBinding);
		~DataBinding() = default;

		constexpr void Init(Device& dev, const DataBindingDesc& desc) { ZE_API_BACKEND_VAR.Init(dev, desc); }
		ZE_API_BACKEND_GET(Resource::DataBinding);
	};
}
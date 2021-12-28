#pragma once
#include "GFX/API/DX11/Material/Schema.h"
#include "GFX/API/DX12/Material/Schema.h"

namespace ZE::GFX::Material
{
	// Context for performing bind operation for given Schema
	struct BindingContext
	{
		U64 BindIndex = 0;
	};

	// Resource bindings used in shaders
	class Schema final
	{
		// There should be 'renderer' bindings, that are appended to the signature,
		// and also there should be a way to just call DataBinding.Bind(Material) to set all the needed stuff.
		// But at the same time we should also set all the renderer stuff, so we need to access renderer data too...
		ZE_API_BACKEND(Material::Schema);

	public:
		Schema() = default;
		constexpr Schema(Device& dev, const SchemaDesc& desc) { ZE_API_BACKEND_VAR.Init(dev, desc); }
		ZE_CLASS_MOVE(Schema);
		~Schema() = default;

		constexpr void Init(Device& dev, const SchemaDesc& desc) { ZE_API_BACKEND_VAR.Init(dev, desc); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, const SchemaDesc& desc) { ZE_API_BACKEND_VAR.Switch(nextApi, dev, desc); }
		ZE_API_BACKEND_GET(Material::Schema);
	};
}
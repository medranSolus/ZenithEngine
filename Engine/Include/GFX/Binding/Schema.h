#pragma once
#include "GFX/API/DX11/Binding/Schema.h"
#include "GFX/API/DX12/Binding/Schema.h"

namespace ZE::GFX::Binding
{
	// Resource bindings used in shaders
	class Schema final
	{
		// There should be 'renderer' bindings, that are appended to the signature,
		// and also there should be a way to just call DataBinding.Bind(Material) to set all the needed stuff.
		// But at the same time we should also set all the renderer stuff, so we need to access renderer data too...
		ZE_API_BACKEND(Binding::Schema);

	public:
		Schema() = default;
		constexpr Schema(Device& dev, const SchemaDesc& desc) { ZE_API_BACKEND_VAR.Init(dev, desc); }
		ZE_CLASS_MOVE(Schema);
		~Schema() = default;

		constexpr void Init(Device& dev, const SchemaDesc& desc) { ZE_API_BACKEND_VAR.Init(dev, desc); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, const SchemaDesc& desc) { ZE_API_BACKEND_VAR.Switch(nextApi, dev, desc); }
		ZE_API_BACKEND_GET(Binding::Schema);

		// Main Gfx API

		constexpr void SetCompute(CommandList& cl) const noexcept { ZE_API_BACKEND_CALL(SetCompute, cl); }
		constexpr void SetGraphics(CommandList& cl) const noexcept { ZE_API_BACKEND_CALL(SetGraphics, cl); }
	};
}
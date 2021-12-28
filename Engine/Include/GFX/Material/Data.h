#pragma once
#include "GFX/API/DX11/Material/Data.h"
#include "GFX/API/DX12/Material/Data.h"

namespace ZE::GFX::Material
{
	// Class containing material data of graphics objects
	class Data final
	{
		ZE_API_BACKEND(Material::Data);

	public:
		constexpr Data(Device& dev, const Schema& schema) { ZE_API_BACKEND_VAR.Init(dev, schema); }
		ZE_CLASS_MOVE(Data);
		~Data() = default;

		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, const Schema& schema) { ZE_API_BACKEND_VAR.Switch(nextApi, dev, schema); }
		ZE_API_BACKEND_GET(Material::Data);
	};
}
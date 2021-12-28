#pragma once
#include "GFX/Material/SchemaDesc.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::DX11::Material
{
	class Schema final
	{
	public:
		Schema(GFX::Device& dev, const GFX::Material::SchemaDesc& desc) {}
		ZE_CLASS_MOVE(Schema);
		~Schema() = default;
	};
}
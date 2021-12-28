#pragma once
#include "GFX/Material/Schema.h"

namespace ZE::GFX::API::DX12::Material
{
	class Data final
	{
	public:
		Data(GFX::Device& dev, const GFX::Material::Schema& schema) {}
		ZE_CLASS_MOVE(Data);
		~Data() = default;
	};
}
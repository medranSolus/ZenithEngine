#pragma once
#include "GFX/Resource/DataBindingDesc.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::DX11::Resource
{
	class DataBinding final
	{
	public:
		DataBinding(GFX::Device& dev, const GFX::Resource::DataBindingDesc& desc) {}
		ZE_CLASS_MOVE(DataBinding);
		~DataBinding() = default;
	};
}
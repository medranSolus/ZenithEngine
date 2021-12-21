#pragma once
#include "GFX/Resource/DataBindingDesc.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::DX12::Resource
{
	class DataBinding final
	{
		DX::ComPtr<ID3D12RootSignature> signature;

	public:
		DataBinding(GFX::Device& dev, const GFX::Resource::DataBindingDesc& desc);
		ZE_CLASS_MOVE(DataBinding);
		~DataBinding() = default;

		// Gfx API Internal

		ID3D12RootSignature* GetSignature() const noexcept { return signature.Get(); }
	};
}
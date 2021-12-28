#pragma once
#include "GFX/Material/SchemaDesc.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::DX12::Material
{
	class Schema final
	{
		DX::ComPtr<ID3D12RootSignature> signature;

	public:
		Schema(GFX::Device& dev, const GFX::Material::SchemaDesc& desc);
		ZE_CLASS_MOVE(Schema);
		~Schema() = default;

		// Gfx API Internal

		ID3D12RootSignature* GetSignature() const noexcept { return signature.Get(); }
	};
}
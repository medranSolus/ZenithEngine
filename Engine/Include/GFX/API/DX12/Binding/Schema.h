#pragma once
#include "GFX/Binding/SchemaDesc.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::DX12::Binding
{
	class Schema final
	{
		DX::ComPtr<ID3D12RootSignature> signature;

	public:
		Schema(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc);
		ZE_CLASS_MOVE(Schema);
		~Schema() = default;

		void SetCompute(GFX::CommandList& cl) const noexcept { cl.Get().dx12.GetList()->SetComputeRootSignature(GetSignature()); }
		void SetGraphics(GFX::CommandList& cl) const noexcept { cl.Get().dx12.GetList()->SetGraphicsRootSignature(GetSignature()); }

		// Gfx API Internal

		ID3D12RootSignature* GetSignature() const noexcept { return signature.Get(); }
	};
}
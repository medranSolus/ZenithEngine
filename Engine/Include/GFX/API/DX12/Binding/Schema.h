#pragma once
#include "GFX/Binding/SchemaDesc.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::DX12::Binding
{
	class Schema final
	{
	public:
		enum class BindType : U8
		{
			Constant, CBV, SRV, UAV, Table
		};

	private:
		bool isCompute;
		Ptr<BindType> bindings;
		DX::ComPtr<ID3D12RootSignature> signature;
#ifdef _ZE_MODE_DEBUG
		U32 count;
#endif

	public:
		Schema() = default;
		Schema(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc);
		ZE_CLASS_MOVE(Schema);
		~Schema();

		void SetCompute(GFX::CommandList& cl) const noexcept { cl.Get().dx12.GetList()->SetComputeRootSignature(GetSignature()); }
		void SetGraphics(GFX::CommandList& cl) const noexcept { cl.Get().dx12.GetList()->SetGraphicsRootSignature(GetSignature()); }

		// Gfx API Internal

		constexpr bool IsCompute() const noexcept { return isCompute; }
		BindType GetCurrentType(U32 index) const noexcept { ZE_ASSERT(index < count, "Access out of range!"); return bindings[index]; }
		ID3D12RootSignature* GetSignature() const noexcept { return signature.Get(); }
	};
}
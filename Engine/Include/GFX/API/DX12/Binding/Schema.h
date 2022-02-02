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
		U32 count;
		Ptr<BindType> bindings;
		DX::ComPtr<ID3D12RootSignature> signature;

	public:
		Schema() = default;
		Schema(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc);
		ZE_CLASS_MOVE(Schema);
		~Schema();

		constexpr U32 GetIndexFromEnd(U32 offsetFromEnd) const noexcept { ZE_ASSERT(count > offsetFromEnd, "Accessing binding out of range!"); return count - offsetFromEnd - 1; }
		void SetCompute(GFX::CommandList& cl) const noexcept { cl.Get().dx12.GetList()->SetComputeRootSignature(GetSignature()); }
		void SetGraphics(GFX::CommandList& cl) const noexcept { cl.Get().dx12.GetList()->SetGraphicsRootSignature(GetSignature()); }

		// Gfx API Internal

		constexpr bool IsCompute() const noexcept { return isCompute; }
		constexpr bool GetCount() const noexcept { return count; }

		BindType GetCurrentType(U32 index) const noexcept { ZE_ASSERT(index < count, "Access out of range!"); return bindings[index]; }
		ID3D12RootSignature* GetSignature() const noexcept { return signature.Get(); }
	};
}
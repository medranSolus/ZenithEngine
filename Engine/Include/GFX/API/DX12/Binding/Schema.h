#pragma once
#include "GFX/Binding/SchemaDesc.h"
#include "GFX/CommandList.h"

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
		DX::ComPtr<IRootSignature> signature;

	public:
		Schema() = default;
		Schema(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc);
		ZE_CLASS_MOVE(Schema);
		~Schema();

		constexpr U32 GetCount() const noexcept { return count; }
		void SetCompute(GFX::CommandList& cl) const noexcept { ZE_ASSERT(isCompute, "Schema is not created for compute pass!"); cl.Get().dx12.GetList()->SetComputeRootSignature(GetSignature()); }
		void SetGraphics(GFX::CommandList& cl) const noexcept { ZE_ASSERT(!isCompute, "Schema is not created for graphics pass!"); cl.Get().dx12.GetList()->SetGraphicsRootSignature(GetSignature()); }

		// Gfx API Internal

		constexpr bool IsCompute() const noexcept { return isCompute; }

		BindType GetCurrentType(U32 index) const noexcept { ZE_ASSERT(index < count, "Access out of range!"); return bindings[index]; }
		IRootSignature* GetSignature() const noexcept { return signature.Get(); }
	};
}
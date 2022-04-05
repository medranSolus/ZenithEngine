#pragma once
#include "GFX/Binding/SchemaDesc.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::DX11::Binding
{
	class Schema final
	{
		std::bitset<6> activeShaders;
		U32 count;
		Ptr<std::pair<GFX::Resource::ShaderTypes, U32>> slots;
		U32 samplersCount;
		Ptr<std::pair<U32, DX::ComPtr<ID3D11SamplerState>>> samplers;

	public:
		Schema() = default;
		Schema(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc);
		ZE_CLASS_MOVE(Schema);
		~Schema();

		constexpr U32 GetCount() const noexcept { return count; }
		void SetCompute(GFX::CommandList& cl) const noexcept;
		void SetGraphics(GFX::CommandList& cl) const noexcept;

		// Gfx API Internal

		std::pair<GFX::Resource::ShaderTypes, U32> GetCurrentSlot(U32 index) const noexcept { ZE_ASSERT(index < count, "Access out of range!"); return slots[index]; }
	};
}
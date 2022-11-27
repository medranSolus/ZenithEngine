#pragma once
#include "GFX/Binding/SchemaDesc.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::DX11::Binding
{
	class Schema final
	{
	public:
		struct SlotInfo
		{
			U32 DataStart;
			U32 SlotsCount;
		};
		struct SlotData
		{
			GFX::Resource::ShaderTypes Shaders;
			U32 BindStart;
			U32 Count;
		};

	private:
		std::bitset<6> activeShaders;
		U32 count;
		Ptr<SlotInfo> slots;
		Ptr<SlotData> slotsData;
		U32 samplersCount;
		Ptr<std::pair<U32, DX::ComPtr<ISamplerState>>> samplers;

	public:
		Schema() = default;
		Schema(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc);
		ZE_CLASS_MOVE(Schema);
		~Schema();

		constexpr U32 GetCount() const noexcept { return count; }
		void SetCompute(GFX::CommandList& cl) const noexcept;
		void SetGraphics(GFX::CommandList& cl) const noexcept;

		// Gfx API Internal

		SlotInfo GetCurrentSlot(U32 index) const noexcept { ZE_ASSERT(index < count, "Access out of range!"); return slots[index]; }
		SlotData GetSlotData(U32 index) const noexcept { return slotsData[index]; }
	};
}
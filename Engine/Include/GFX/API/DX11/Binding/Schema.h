#pragma once
#include "GFX/Binding/SchemaDesc.h"
#include "GFX/CommandList.h"
#include "GFX/ShaderPresence.h"

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
		ShaderPresenceMask activeShaders;
		U32 count;
		Ptr<SlotInfo> slots;
		Ptr<SlotData> slotsData;
		U32 samplersCount;
		Ptr<std::pair<U32, DX::ComPtr<ISamplerState>>> samplers;

	public:
		Schema() = default;
		Schema(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc);
		ZE_CLASS_MOVE(Schema);
		~Schema() { ZE_ASSERT(slots == nullptr && slotsData == nullptr && samplers == nullptr, "Resource not freed before deletion!"); }

		constexpr U32 GetCount() const noexcept { return count; }

		void Free(GFX::Device& dev) noexcept;
		void SetCompute(GFX::CommandList& cl) const noexcept;
		void SetGraphics(GFX::CommandList& cl) const noexcept;

		// Gfx API Internal

		SlotInfo GetCurrentSlot(U32 index) const noexcept { ZE_ASSERT(index < count, "Access out of range!"); return slots[index]; }
		SlotData GetSlotData(U32 index) const noexcept { return slotsData[index]; }
	};
}
#pragma once
#include "GFX/Binding/SchemaDesc.h"
#include "GFX/CommandList.h"
#include "GFX/ShaderPresence.h"

namespace ZE::RHI::DX11::Binding
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
		GFX::ShaderPresenceMask activeShaders;
		U32 count = 0;
		std::unique_ptr<SlotInfo[]> slots;
		std::unique_ptr<SlotData[]> slotsData;
		U32 samplersCount = 0;
		std::unique_ptr<std::pair<U32, DX::ComPtr<ISamplerState>>[]> samplers;

	public:
		Schema() = default;
		ZE_CLASS_MOVE(Schema);
		~Schema() = default;

		static Expected<Schema> Create(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc) noexcept;

		constexpr U32 GetCount() const noexcept { return count; }

		void SetCompute(GFX::CommandList& cl) const noexcept;
		void SetGraphics(GFX::CommandList& cl) const noexcept;

		// Gfx API Internal

		SlotInfo GetCurrentSlot(U32 index) const noexcept { ZE_ASSERT(index < count, "Access out of range!"); return slots[index]; }
		SlotData GetSlotData(U32 index) const noexcept { return slotsData[index]; }
	};
}
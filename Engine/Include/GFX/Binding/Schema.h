#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/Binding/Schema.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/Binding/Schema.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/Binding/Schema.h"
#endif

namespace ZE::GFX::Binding
{
	// Resource bindings used in shaders
	class Schema final
	{
		ZE_RHI_BACKEND(Binding::Schema);

	public:
		Schema() = default;
		ZE_CLASS_MOVE(Schema);
		~Schema() = default;

		static Expected<Schema> Create(Device& dev, const SchemaDesc& desc) noexcept { ZE_RHI_BACKEND_CREATE(Binding::Schema, dev, desc); }
		ZE_RHI_BACKEND_GET(Binding::Schema);

		// Main Gfx API

		// Get binding index from with offset from the end (0=last, 1=last-1, etc.)
		constexpr U32 GetIndexFromEnd(U32 offsetFromEnd) const noexcept { U32 count = 0; ZE_RHI_BACKEND_CALL_RET_VAR(count, GetCount); ZE_ASSERT(count > offsetFromEnd, "Accessing binding out of range!"); return count - offsetFromEnd - 1; }
		constexpr void SetCompute(CommandList& cl) const noexcept { ZE_RHI_BACKEND_CALL(SetCompute, cl); }
		constexpr void SetGraphics(CommandList& cl) const noexcept { ZE_RHI_BACKEND_CALL(SetGraphics, cl); }
	};
}
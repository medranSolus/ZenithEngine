#pragma once
#include "GFX/API/DX11/Resource/CBuffer.h"

namespace ZE::GFX::Resource
{
	// Constant shader buffer
	class CBuffer final
	{
		ZE_API_BACKEND(CBuffer);

	public:
		constexpr CBuffer(Device& dev, U8* values, U32 bytes, bool dynamic) { ZE_API_BACKEND_VAR.Init(dev, values, bytes, dynamic); }
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() = default;

		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, Context& ctx, U8* values, U64 bytes) { ZE_API_BACKEND_VAR.Switch(nextApi, dev, values, bytes); }
		ZE_API_BACKEND_GET(CBuffer);

		// Main Gfx API

		constexpr void Update(CommanList& cl, U8* values, U64 bytes) const { ZE_API_BACKEND_CALL(Update, cl, values, bytes); }
		constexpr void UpdateDynamic(Device& dev, CommandList& cl, U8* values, U64 bytes) const { ZE_API_BACKEND_CALL(UpdateDynamic, dev, cl, values, bytes); }

		constexpr void BindVS(CommanList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(BindVS, cl, slot); }
		constexpr void BindDS(CommanList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(BindDS, cl, slot); }
		constexpr void BindHS(CommanList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(BindHS, cl, slot); }
		constexpr void BindGS(CommanList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(BindGS, cl, slot); }
		constexpr void BindPS(CommanList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(BindPS, cl, slot); }
		constexpr void BindCS(CommanList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(BindCS, cl, slot); }
	};
}
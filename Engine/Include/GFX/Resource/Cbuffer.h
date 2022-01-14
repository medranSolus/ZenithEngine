#pragma once
#include "GFX/API/DX11/Resource/CBuffer.h"
#include "GFX/API/DX12/Resource/CBuffer.h"

namespace ZE::GFX::Resource
{
	// Constant shader buffer
	class CBuffer final
	{
		ZE_API_BACKEND(Resource::CBuffer);

	public:
		CBuffer() = default;
		constexpr CBuffer(Device& dev, const U8* values, U32 bytes, bool dynamic) { ZE_API_BACKEND_VAR.Init(dev, values, bytes, dynamic); }
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() = default;

		constexpr void Init(Device& dev, const U8* values, U32 bytes, bool dynamic) { ZE_API_BACKEND_VAR.Init(dev, values, bytes, dynamic); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, U8* values, U32 bytes, bool dynamic) { ZE_API_BACKEND_VAR.Switch(nextApi, dev, values, bytes, dynamic); }
		ZE_API_BACKEND_GET(Resource::CBuffer);

		// Main Gfx API

		constexpr void Update(CommandList& cl, const U8* values, U32 bytes) const { ZE_API_BACKEND_CALL(Update, cl, values, bytes); }
		constexpr void UpdateDynamic(Device& dev, CommandList& cl, const U8* values, U32 bytes) const { ZE_API_BACKEND_CALL(UpdateDynamic, dev, cl, values, bytes); }

		constexpr void BindVS(CommandList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(BindVS, cl, slot); }
		constexpr void BindDS(CommandList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(BindDS, cl, slot); }
		constexpr void BindHS(CommandList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(BindHS, cl, slot); }
		constexpr void BindGS(CommandList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(BindGS, cl, slot); }
		constexpr void BindPS(CommandList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(BindPS, cl, slot); }
		constexpr void BindCS(CommandList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(BindCS, cl, slot); }
	};
}
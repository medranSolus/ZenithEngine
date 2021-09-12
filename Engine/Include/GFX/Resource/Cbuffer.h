#pragma once
#include "GFX/API/DX11/Resource/CBuffer.h"

namespace ZE::GFX::Resource
{
	// Constant shader buffer
	class CBuffer final
	{
		ZE_API_BACKEND(CBuffer) backend;

	public:
		constexpr CBuffer(Device& dev, U8* values, U32 bytes, bool dynamic) { backend.Init(dev, values, bytes, dynamic); }
		CBuffer(CBuffer&&) = default;
		CBuffer(const CBuffer&) = delete;
		CBuffer& operator=(CBuffer&&) = default;
		CBuffer& operator=(const CBuffer&) = delete;
		~CBuffer() = default;

		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, Context& ctx, U8* values, U64 bytes) { backend.Switch(nextApi, dev, values, bytes); }
		constexpr ZE_API_BACKEND(CBuffer)& Get() noexcept { return backend; }

		// Main Gfx API

		constexpr void Update(CommanList& cl, U8* values, U64 bytes) const { ZE_API_BACKEND_CALL(backend, Update, cl, values, bytes); }
		constexpr void UpdateDynamic(Device& dev, CommandList& cl, U8* values, U64 bytes) const { ZE_API_BACKEND_CALL(backend, UpdateDynamic, dev, cl, values, bytes); }

		constexpr void BindVS(CommanList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(backend, BindVS, cl, slot); }
		constexpr void BindDS(CommanList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(backend, BindDS, cl, slot); }
		constexpr void BindHS(CommanList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(backend, BindHS, cl, slot); }
		constexpr void BindGS(CommanList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(backend, BindGS, cl, slot); }
		constexpr void BindPS(CommanList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(backend, BindPS, cl, slot); }
		constexpr void BindCS(CommanList& cl, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(backend, BindCS, cl, slot); }
	};
}
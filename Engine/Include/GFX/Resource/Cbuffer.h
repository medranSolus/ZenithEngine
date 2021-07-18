#pragma once
#include "GFX/API/DX11/Resource/CBuffer.h"

namespace ZE::GFX::Resource
{
	// Constant shader buffer
	class CBuffer final
	{
		ZE_API_BACKEND(CBuffer, DX11, DX11, DX11, DX11) backend;

	public:
		constexpr CBuffer(Device& dev, U8* values, U32 bytes, bool dynamic) { backend.Init(dev, values, bytes, dynamic); }
		CBuffer(CBuffer&&) = delete;
		CBuffer(const CBuffer&) = delete;
		CBuffer& operator=(CBuffer&&) = delete;
		CBuffer& operator=(const CBuffer&) = delete;
		~CBuffer() = default;

		template<bool IsDynamic>
		constexpr void Update(GFX::Device& dev, GFX::Context& ctx, U8* values, U64 bytes) const { ZE_API_BACKEND_CALL(backend, Update<IsDynamic>, dev, ctx, values, bytes); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, Context& ctx, U8* values, U64 bytes) { backend.Switch(nextApi, dev, values, bytes); }
		constexpr ZE_API_BACKEND(CBuffer, DX11, DX11, DX11, DX11)& Get() noexcept { return backend; }

		void BindVS(Context& ctx, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(backend, BindVS, ctx, slot); }
		void BindDS(Context& ctx, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(backend, BindDS, ctx, slot); }
		void BindHS(Context& ctx, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(backend, BindHS, ctx, slot); }
		void BindGS(Context& ctx, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(backend, BindGS, ctx, slot); }
		void BindPS(Context& ctx, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(backend, BindPS, ctx, slot); }
		void BindCS(Context& ctx, ShaderSlot slot) const noexcept { ZE_API_BACKEND_CALL(backend, BindCS, ctx, slot); }
	};
}
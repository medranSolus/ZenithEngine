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
		constexpr CBuffer(Device& dev, const void* values, U32 bytes, bool dynamic) { ZE_API_BACKEND_VAR.Init(dev, values, bytes, dynamic); }
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() = default;

		constexpr void Init(Device& dev, const void* values, U32 bytes, bool dynamic) { ZE_API_BACKEND_VAR.Init(dev, values, bytes, dynamic); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, void* values, U32 bytes, bool dynamic) { ZE_API_BACKEND_VAR.Switch(nextApi, dev, values, bytes, dynamic); }
		ZE_API_BACKEND_GET(Resource::CBuffer);

		// Main Gfx API

		// Requires call to Device::StartUpload() if created buffer is no dynamic
		constexpr void Update(Device& dev, CommandList& cl, const void* values, U32 bytes) const { ZE_API_BACKEND_CALL(Update, dev, cl, values, bytes); }
		constexpr void Bind(CommandList& cl, Binding::Context& bindCtx) const noexcept { ZE_API_BACKEND_CALL(Bind, cl, bindCtx); }
	};
}
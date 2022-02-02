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
		// Requires call to Device::StartUpload() if created buffer is not dynamic or 'values' is not nullptr
		constexpr CBuffer(Device& dev, const void* values, U32 bytes, bool dynamic) { ZE_API_BACKEND_VAR.Init(dev, values, bytes, dynamic); }
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() = default;

		// Requires call to Device::StartUpload() if created buffer is not dynamic or 'values' is not nullptr
		constexpr void Init(Device& dev, const void* values, U32 bytes, bool dynamic) { ZE_API_BACKEND_VAR.Init(dev, values, bytes, dynamic); }
		// Requires call to Device::StartUpload() if created buffer is not dynamic or 'values' is not nullptr
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, void* values, U32 bytes, bool dynamic) { ZE_API_BACKEND_VAR.Switch(nextApi, dev, values, bytes, dynamic); }
		ZE_API_BACKEND_GET(Resource::CBuffer);

		// Main Gfx API

		// Get pointer to region of a buffer. Only valid on dynamic buffers
		constexpr void* GetRegion() const noexcept { void* buffer = nullptr; ZE_API_BACKEND_CALL_RET(buffer, GetRegion); return buffer; }
		// Requires call to Device::StartUpload() if created buffer is not dynamic or 'values' is not nullptr
		constexpr void Update(Device& dev, CommandList& cl, const void* values, U32 bytes) const { ZE_API_BACKEND_CALL(Update, dev, cl, values, bytes); }
		constexpr void Bind(CommandList& cl, Binding::Context& bindCtx) const noexcept { ZE_API_BACKEND_CALL(Bind, cl, bindCtx); }
		// Before destroying buffer you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_API_BACKEND_CALL(Free, dev); }
	};
}
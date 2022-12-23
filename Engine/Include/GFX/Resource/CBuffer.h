#pragma once
#include "GFX/API/DX11/Resource/CBuffer.h"
#include "GFX/API/DX12/Resource/CBuffer.h"
#include "GFX/API/VK/Resource/CBuffer.h"

namespace ZE::GFX::Resource
{
	// Constant shader buffer
	class CBuffer final
	{
		ZE_API_BACKEND(Resource::CBuffer);

	public:
		CBuffer() = default;
		// Requires call to Device::BeginUploadRegion() if 'values' is not nullptr
		constexpr CBuffer(Device& dev, const void* values, U32 bytes) { Init(dev, values, bytes); }
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() = default;

		// Requires call to Device::BeginUploadRegion() if 'values' is not nullptr
		constexpr void Init(Device& dev, const void* values, U32 bytes) { ZE_ASSERT(bytes != 0, "Empty buffer!"); ZE_API_BACKEND_VAR.Init(dev, values, bytes); }
		// Requires call to Device::BeginUploadRegion() if 'values' is not nullptr
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, void* values, U32 bytes, bool dynamic) { ZE_API_BACKEND_VAR.Switch(nextApi, dev, values, bytes); }
		ZE_API_BACKEND_GET(Resource::CBuffer);

		// Main Gfx API

		// Requires call to Device::BeginUploadRegion() if 'values' is not nullptr
		constexpr void Update(Device& dev, const void* values, U32 bytes) const { ZE_ASSERT(values && bytes, "Empty buffer!"); ZE_API_BACKEND_CALL(Update, dev, values, bytes); }
		constexpr void Bind(CommandList& cl, Binding::Context& bindCtx) const noexcept { ZE_API_BACKEND_CALL(Bind, cl, bindCtx); }
		// Before destroying buffer you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_API_BACKEND_CALL(Free, dev); }
	};
}
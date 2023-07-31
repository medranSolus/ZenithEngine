#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/Resource/CBuffer.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/Resource/CBuffer.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/Resource/CBuffer.h"
#endif

namespace ZE::GFX::Resource
{
	// Constant shader buffer
	class CBuffer final
	{
		ZE_RHI_BACKEND(Resource::CBuffer);

	public:
		CBuffer() = default;
		// Requires call to Device::BeginUploadRegion() if 'values' is not nullptr
		constexpr CBuffer(Device& dev, const void* values, U32 bytes) { Init(dev, values, bytes); }
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() = default;

		// Requires call to Device::BeginUploadRegion() if 'values' is not nullptr
		constexpr void Init(Device& dev, const void* values, U32 bytes) { ZE_ASSERT(bytes != 0, "Empty buffer!"); ZE_RHI_BACKEND_VAR.Init(dev, values, bytes); }
		// Requires call to Device::BeginUploadRegion() if 'values' is not nullptr
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, void* values, U32 bytes, bool dynamic) { ZE_RHI_BACKEND_VAR.Switch(nextApi, dev, values, bytes); }
		ZE_RHI_BACKEND_GET(Resource::CBuffer);

		// Main Gfx API

		// Requires call to Device::BeginUploadRegion() if 'values' is not nullptr
		constexpr void Update(Device& dev, const void* values, U32 bytes) const { ZE_ASSERT(values && bytes, "Empty buffer!"); ZE_RHI_BACKEND_CALL(Update, dev, values, bytes); }
		constexpr void Bind(CommandList& cl, Binding::Context& bindCtx) const noexcept { ZE_RHI_BACKEND_CALL(Bind, cl, bindCtx); }
		// Before destroying buffer you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_RHI_BACKEND_CALL(Free, dev); }
	};
}
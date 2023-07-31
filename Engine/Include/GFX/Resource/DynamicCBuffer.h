#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/Resource/DynamicCBuffer.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/Resource/DynamicCBuffer.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/Resource/DynamicCBuffer.h"
#endif

namespace ZE::GFX::Resource
{
	// Dynamic CBuffer holding data changing every frame inside big chunks of memory
	class DynamicCBuffer final
	{
		ZE_RHI_BACKEND(Resource::DynamicCBuffer);

	public:
		DynamicCBuffer() = default;
		constexpr DynamicCBuffer(Device& dev) { ZE_RHI_BACKEND_VAR.Init(dev); }
		ZE_CLASS_MOVE(DynamicCBuffer);
		~DynamicCBuffer() = default;

		constexpr void Init(Device& dev) { ZE_RHI_BACKEND_VAR.Init(dev); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev) { ZE_RHI_BACKEND_VAR.Switch(nextApi, dev); }
		ZE_RHI_BACKEND_GET(Resource::DynamicCBuffer);

		// Main Gfx API

		constexpr DynamicBufferAlloc Alloc(Device& dev, const void* values, U32 bytes) { DynamicBufferAlloc info; ZE_RHI_BACKEND_CALL_RET(info, Alloc, dev, values, bytes); return info; }
		constexpr void Bind(CommandList& cl, Binding::Context& bindCtx, const DynamicBufferAlloc& allocInfo) const noexcept { ZE_RHI_BACKEND_CALL(Bind, cl, bindCtx, allocInfo); }
		constexpr void AllocBind(Device& dev, CommandList& cl, Binding::Context& bindCtx, const void* values, U32 bytes) { Bind(cl, bindCtx, Alloc(dev, values, bytes)); }
		constexpr void StartFrame(Device& dev) { ZE_RHI_BACKEND_CALL(StartFrame, dev); }
		constexpr void Free(Device& dev) noexcept { ZE_RHI_BACKEND_CALL(Free, dev); }
	};
}
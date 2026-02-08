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
		ZE_CLASS_MOVE(DynamicCBuffer);
		~DynamicCBuffer() = default;

		static Expected<DynamicCBuffer> Create(Device& dev) noexcept { ZE_RHI_BACKEND_CREATE(Resource::DynamicCBuffer, dev); }
		ZE_RHI_BACKEND_GET(Resource::DynamicCBuffer);

		// Main Gfx API

		constexpr Expected<DynamicBufferAlloc> Alloc(Device& dev, const void* values, U32 bytes) noexcept { ZE_RHI_BACKEND_CALL_RET(Alloc, dev, values, bytes); }
		constexpr void Bind(CommandList& cl, Binding::Context& bindCtx, const DynamicBufferAlloc& allocInfo) const noexcept { ZE_RHI_BACKEND_CALL(Bind, cl, bindCtx, allocInfo); }
		Status StartFrame(Device& dev) noexcept { ZE_RHI_BACKEND_CALL_RET(StartFrame, dev); }

		Status AllocBind(Device& dev, CommandList& cl, Binding::Context& bindCtx, const void* values, U32 bytes) noexcept;
	};

#pragma region Functions
	inline Status DynamicCBuffer::AllocBind(Device& dev, CommandList& cl, Binding::Context& bindCtx, const void* values, U32 bytes) noexcept
	{
		Expected<DynamicBufferAlloc> alloc = Alloc(dev, values, bytes);
		if (!alloc)
			return alloc.error();
		Bind(cl, bindCtx, alloc.value());
		return {};
	}
#pragma endregion
}
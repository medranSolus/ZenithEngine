#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/Resource/Generic.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/Resource/Generic.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/Resource/Generic.h"
#endif

namespace ZE::GFX::Resource
{
	// Custom generic resource that can hold various types of data inside, depending on requested params
	class Generic final
	{
		ZE_RHI_BACKEND(Resource::Generic);

	public:
		Generic() = default;
		constexpr Generic(Device& dev, const GenericResourceDesc& desc) { Init(dev, desc); }
		constexpr Generic(Pipeline::FrameBuffer& framebuffer, RID rid) noexcept { Init(framebuffer, rid); }
		ZE_CLASS_MOVE(Generic);
		~Generic() = default;

		constexpr void Init(Device& dev, const GenericResourceDesc& desc) { ZE_RHI_BACKEND_VAR.Init(dev, desc); }
		constexpr void Init(Pipeline::FrameBuffer& framebuffer, RID rid) noexcept { ZE_RHI_BACKEND_VAR.Init(framebuffer, rid); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, const GenericResourceDesc& desc) { ZE_RHI_BACKEND_VAR.Switch(nextApi, dev, desc); }
		ZE_RHI_BACKEND_GET(Resource::Generic);

		// Main Gfx API

		constexpr U8* GetBuffer() noexcept { U8* ptr = nullptr; ZE_RHI_BACKEND_CALL_RET(ptr, GetBuffer); return ptr; }
		constexpr bool IsStagingCopyRequired(Device& dev, const GenericResourceDesc& desc) const noexcept { bool val = true; ZE_RHI_BACKEND_CALL_RET(val, IsStagingCopyRequired, dev, desc); return val; }
		constexpr void ClearUAV(CommandList& cl, const ColorF4& color) const noexcept { ZE_RHI_BACKEND_CALL(ClearUAV, cl, color); }
		constexpr void Copy(Device& dev, CommandList& cl, Generic& dest) const noexcept { ZE_RHI_BACKEND_CALL(Copy, dev, cl, dest); }
		constexpr void Bind(Device& dev, CommandList& cl, Binding::Context& bindCtx, bool uav, U16 uavMipLevel = 0) const noexcept { ZE_RHI_BACKEND_CALL(Bind, dev, cl, bindCtx, uav, uavMipLevel); }
		constexpr void ExecuteIndirectCommands(CommandList& cl, CommandSignature& signature, U32 commandOffset) const noexcept { ZE_RHI_BACKEND_CALL(ExecuteIndirectCommands, cl, signature, commandOffset); }
		// Before destroying buffer you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_RHI_BACKEND_CALL(Free, dev); }
	};
}
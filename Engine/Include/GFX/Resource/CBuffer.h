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
		constexpr CBuffer(Device& dev, DiskManager& disk, const CBufferData& data) { Init(dev, disk, data); }
		constexpr CBuffer(Device& dev, DiskManager& disk, const CBufferFileData& data, GFile& file) { Init(dev, disk, data, file); }
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() = default;

		constexpr void Init(Device& dev, DiskManager& disk, const CBufferData& data) { ZE_ASSERT((data.DataStatic || data.DataRef) && data.Bytes, "Empty buffer!"); ZE_RHI_BACKEND_VAR.Init(dev, disk, data); }
		constexpr void Init(Device& dev, DiskManager& disk, const CBufferFileData& data, GFile& file) { ZE_ASSERT(data.SourceBytes, "Empty buffer!"); ZE_RHI_BACKEND_VAR.Init(dev, disk, data, file); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, DiskManager& disk, const CBufferData& data) { ZE_RHI_BACKEND_VAR.Switch(nextApi, dev, disk, data); }
		ZE_RHI_BACKEND_GET(Resource::CBuffer);

		// Main Gfx API

		constexpr void Update(Device& dev, DiskManager& disk, const CBufferData& data) const { ZE_ASSERT((data.DataStatic || data.DataRef) && data.Bytes, "Empty buffer!"); ZE_RHI_BACKEND_CALL(Update, dev, disk, data); }
		constexpr void Bind(CommandList& cl, Binding::Context& bindCtx) const noexcept { ZE_RHI_BACKEND_CALL(Bind, cl, bindCtx); }
		// Before destroying buffer you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_RHI_BACKEND_CALL(Free, dev); }
	};
}
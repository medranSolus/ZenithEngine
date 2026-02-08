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
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() = default;

		static Expected<CBuffer> Create(Device& dev, DiskManager& disk, const CBufferData& data) noexcept { ZE_ASSERT((data.DataStatic || data.DataRef) && data.Bytes, "Empty buffer!"); ZE_RHI_BACKEND_CREATE(Resource::CBuffer, dev, disk, data); }
		static Expected<CBuffer> Create(Device& dev, DiskManager& disk, const CBufferFileData& data, GFile& file) noexcept { ZE_ASSERT(data.SourceBytes, "Empty buffer!"); ZE_RHI_BACKEND_CREATE(Resource::CBuffer, dev, disk, data, file); }
		ZE_RHI_BACKEND_GET(Resource::CBuffer);

		// Main Gfx API

		constexpr void Bind(CommandList& cl, Binding::Context& bindCtx) const noexcept { ZE_RHI_BACKEND_CALL(Bind, cl, bindCtx); }
		Status Update(Device& dev, DiskManager& disk, const CBufferData& data) const noexcept { ZE_ASSERT((data.DataStatic || data.DataRef) && data.Bytes, "Empty buffer!"); ZE_RHI_BACKEND_CALL_RET(Update, dev, disk, data); }
	};
}
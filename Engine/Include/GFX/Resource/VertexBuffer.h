#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/Resource/VertexBuffer.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/Resource/VertexBuffer.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/Resource/VertexBuffer.h"
#endif

namespace ZE::GFX::Resource
{
	// Buffer holding vertex data
	class VertexBuffer final
	{
		ZE_RHI_BACKEND(Resource::VertexBuffer);

	public:
		VertexBuffer() = default;
		constexpr VertexBuffer(Device& dev, const VertexData& data) { Init(dev, data); }
		ZE_CLASS_MOVE(VertexBuffer);
		~VertexBuffer() = default;

		constexpr void Init(Device& dev, const VertexData& data) { ZE_ASSERT(data.Vertices && data.Count && data.VertexSize, "Empty vertex data!"); ZE_RHI_BACKEND_VAR.Init(dev, data); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, CommandList& cl);
		ZE_RHI_BACKEND_GET(Resource::VertexBuffer);

		// Main Gfx API

		constexpr void Bind(CommandList& cl) const noexcept { ZE_RHI_BACKEND_CALL(Bind, cl); }
		// Before destroying buffer you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_RHI_BACKEND_CALL(Free, dev); }
	};

#pragma region Functions
	constexpr void VertexBuffer::SwitchApi(GfxApiType nextApi, Device& dev, CommandList& cl)
	{
		VertexData data;
		ZE_RHI_BACKEND_CALL_RET(data, GetData, dev, cl);
		ZE_RHI_BACKEND_VAR.Switch(nextApi, dev, data);
	}
#pragma endregion
}
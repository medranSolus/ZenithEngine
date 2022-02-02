#pragma once
#include "GFX/API/DX11/Resource/IndexBuffer.h"
#include "GFX/API/DX12/Resource/IndexBuffer.h"

namespace ZE::GFX::Resource
{
	// Buffer holding indices into VertexBuffer
	class IndexBuffer final
	{
		ZE_API_BACKEND(Resource::IndexBuffer);

	public:
		constexpr IndexBuffer(Device& dev, const IndexData& data) { ZE_API_BACKEND_VAR.Init(dev, data); }
		ZE_CLASS_MOVE(IndexBuffer);
		~IndexBuffer() = default;

		ZE_API_BACKEND_GET(Resource::IndexBuffer);
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, CommandList& cl);

		// Main Gfx API

		constexpr U32 GetCount() const noexcept { U32 count = 0; ZE_API_BACKEND_CALL_RET(count, GetCount); return count; }
		constexpr void Bind(CommandList& cl) const noexcept { ZE_API_BACKEND_CALL(Bind, cl); }
		// Before destroying buffer you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_API_BACKEND_CALL(Free, dev); }
	};

#pragma region Functions
	constexpr void IndexBuffer::SwitchApi(GfxApiType nextApi, Device& dev, CommandList& cl)
	{
		IndexData data;
		ZE_API_BACKEND_CALL_RET(data, GetData, dev, cl);
		ZE_API_BACKEND_VAR.Switch(nextApi, dev, data);
	}
#pragma endregion
}
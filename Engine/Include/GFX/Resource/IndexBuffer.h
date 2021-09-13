#pragma once
#include "GFX/API/DX11/Resource/IndexBuffer.h"
#include "GFX/API/DX12/Resource/IndexBuffer.h"

namespace ZE::GFX::Resource
{
	// Buffer holding indices into VertexBuffer
	class IndexBuffer final
	{
		ZE_API_BACKEND(Resource::IndexBuffer) backend;

	public:
		constexpr IndexBuffer(Device& dev, const IndexData& data) { backend.Init(dev, data); }
		IndexBuffer(IndexBuffer&&) = default;
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(IndexBuffer&&) = default;
		IndexBuffer& operator=(const IndexBuffer&) = delete;
		~IndexBuffer() = default;

		constexpr ZE_API_BACKEND(Resource::IndexBuffer)& Get() noexcept { return backend; }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, CommandList& cl);

		// Main Gfx API

		constexpr U32 GetCount() const noexcept { U32 count = 0; ZE_API_BACKEND_CALL_RET(backend, count, GetCount); return count; }
		constexpr void Free(Device& dev) noexcept { ZE_API_BACKEND_CALL(backend, Free, dev); }
		constexpr void Bind(CommandList& cl) const noexcept { ZE_API_BACKEND_CALL(backend, Bind, cl); }
	};

#pragma region Functions
	constexpr void IndexBuffer::SwitchApi(GfxApiType nextApi, Device& dev, CommandList& cl)
	{
		IndexData data;
		ZE_API_BACKEND_CALL_RET(backend, data, GetData, dev, cl);
		backend.Switch(nextApi, dev, data);
	}
#pragma endregion
}
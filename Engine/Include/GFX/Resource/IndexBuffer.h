#pragma once
#include "GFX/API/DX11/Resource/IndexBuffer.h"

namespace ZE::GFX::Resource
{
	// Buffer holding indices into VertexBuffer
	class IndexBuffer final
	{
		ZE_API_BACKEND(IndexBuffer, DX11, DX11, DX11, DX11) backend;
		U32 count;

	public:
		constexpr IndexBuffer(Device& dev, U32 count, U32* indices) : count(count) { backend.Init(dev, count, indices); }
		IndexBuffer(IndexBuffer&&) = delete;
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(IndexBuffer&&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;
		~IndexBuffer() = default;

		constexpr ZE_API_BACKEND(IndexBuffer, DX11, DX11, DX11, DX11)& Get() noexcept { return backend; }
		constexpr U32 GetCount() const noexcept { return count; }
		void Bind(Context& ctx) const noexcept { ZE_API_BACKEND_CALL(backend, Bind, ctx); }

		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, Context& ctx);
	};

#pragma region Functions
	constexpr void IndexBuffer::SwitchApi(GfxApiType nextApi, Device& dev, Context& ctx)
	{
		U32* indices = nullptr;
		ZE_API_BACKEND_CALL_RET(backend, indices, GetData, dev, ctx);
		backend.Switch(nextApi, dev, count, indices);
	}
#pragma endregion
}
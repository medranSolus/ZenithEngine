#pragma once
#include "GFX/API/DX11/Resource/VertexBuffer.h"
#include "GFX/API/DX12/Resource/VertexBuffer.h"

namespace ZE::GFX::Resource
{
	// Buffer holding vertex data
	class VertexBuffer final
	{
		ZE_API_BACKEND(VertexBuffer) backend;

	public:
		constexpr VertexBuffer(Device& dev, const VertexData& data) { backend.Init(dev, data); }
		VertexBuffer(VertexBuffer&&) = default;
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(VertexBuffer&&) = default;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
		~VertexBuffer() = default;

		constexpr ZE_API_BACKEND(IndexBuffer)& Get() noexcept { return backend; }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, Context& ctx);

		// Main Gfx API

		constexpr void Free(Device& dev) noexcept { ZE_API_BACKEND_CALL(backend, Free, dev); }
		constexpr void Bind(CommandList& cl) const noexcept { ZE_API_BACKEND_CALL(backend, Bind, cl); }
	};

#pragma region Functions
	constexpr void VertexBuffer::SwitchApi(GfxApiType nextApi, Device& dev, Context& ctx)
	{
		VertexData data;
		ZE_API_BACKEND_CALL_RET(backend, data, GetData, dev, ctx);
		backend.Switch(nextApi, dev, data);
	}
#pragma endregion
}
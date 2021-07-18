#pragma once
#include "GFX/API/DX11/Resource/VertexBuffer.h"

namespace ZE::GFX::Resource
{
	// Buffer holding vertex data
	class VertexBuffer final
	{
		ZE_API_BACKEND(VertexBuffer, DX11, DX11, DX11, DX11) backend;

	public:
		constexpr VertexBuffer(Device& dev, const VertexData& data) { backend.Init(dev, data); }
		VertexBuffer(VertexBuffer&&) = delete;
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(VertexBuffer&&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
		~VertexBuffer() = default;

		constexpr ZE_API_BACKEND(IndexBuffer, DX11, DX11, DX11, DX11)& Get() noexcept { return backend; }
		void Bind(Context& ctx) const noexcept { ZE_API_BACKEND_CALL(backend, Bind, ctx); }

		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, Context& ctx);
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
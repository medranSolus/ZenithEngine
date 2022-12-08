#pragma once
#include "GFX/VertexData.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::DX12::Resource
{
	class VertexBuffer final
	{
		D3D12_VERTEX_BUFFER_VIEW view;
		ResourceInfo info;

	public:
		VertexBuffer() = default;
		VertexBuffer(GFX::Device& dev, const VertexData& data);
		ZE_CLASS_MOVE(VertexBuffer);
		~VertexBuffer() { ZE_ASSERT(info.IsFree(), "Resource not freed before deletion!"); }

		void Bind(GFX::CommandList& cl) const noexcept { cl.Get().dx12.GetList()->IASetVertexBuffers(0, 1, &view); }
		void Free(GFX::Device& dev) noexcept { dev.Get().dx12.FreeBuffer(info); }

		VertexData GetData(GFX::Device& dev, GFX::CommandList& ctx) const;
	};
}
#pragma once
#include "GFX/CommandList.h"
#include "GFX/VertexData.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Resource
{
	class VertexBuffer final
	{
		D3D12_VERTEX_BUFFER_VIEW view;
		ResourceInfo info;

	public:
		VertexBuffer(GFX::Device& dev, const VertexData& data);
		VertexBuffer(VertexBuffer&&) = default;
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(VertexBuffer&&) = default;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
		~VertexBuffer() = default;

		void Free(GFX::Device& dev) noexcept { dev.Get().dx12.FreeBuffer(info, view.SizeInBytes); }
		void Bind(GFX::CommandList& cl) const noexcept { cl.Get().dx12.GetList()->IASetVertexBuffers(0, 1, &view); }

		VertexData GetData(GFX::Device& dev, GFX::CommandList& ctx) const;
	};
}
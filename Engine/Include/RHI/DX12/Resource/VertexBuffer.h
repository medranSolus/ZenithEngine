#pragma once
#include "GFX/CommandList.h"
#include "GFX/VertexData.h"

namespace ZE::RHI::DX12::Resource
{
	class VertexBuffer final
	{
		D3D12_VERTEX_BUFFER_VIEW view;
		ResourceInfo info;

	public:
		VertexBuffer() = default;
		VertexBuffer(GFX::Device& dev, const GFX::VertexData& data);
		ZE_CLASS_MOVE(VertexBuffer);
		~VertexBuffer() { ZE_ASSERT(info.IsFree(), "Resource not freed before deletion!"); }

		void Bind(GFX::CommandList& cl) const noexcept { cl.Get().dx12.GetList()->IASetVertexBuffers(0, 1, &view); }
		void Free(GFX::Device& dev) noexcept { dev.Get().dx12.FreeBuffer(info); }

		GFX::VertexData GetData(GFX::Device& dev, GFX::CommandList& ctx) const;
	};
}
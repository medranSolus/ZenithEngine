#pragma once
#include "GFX/CommandList.h"
#include "GFX/VertexData.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Resource
{
	class VertexBuffer final
	{
	public:
		VertexBuffer(GFX::Device& dev, const VertexData& data);
		VertexBuffer(VertexBuffer&&) = default;
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(VertexBuffer&&) = default;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
		~VertexBuffer() = default;

		void Bind(GFX::CommandList& ctx) const noexcept;
		VertexData GetData(GFX::Device& dev, GFX::CommandList& ctx) const;
	};
}
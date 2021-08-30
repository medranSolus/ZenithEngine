#pragma once
#include "GFX/CommandList.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Resource
{
	class IndexBuffer final
	{
	public:
		IndexBuffer(GFX::Device& dev, U32 count, U32* indices);
		IndexBuffer(IndexBuffer&&) = default;
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(IndexBuffer&&) = default;
		IndexBuffer& operator=(const IndexBuffer&) = delete;
		~IndexBuffer() = default;

		void Bind(GFX::CommandList& cl) const noexcept;
		U32* GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}
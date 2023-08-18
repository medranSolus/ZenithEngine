#pragma once
#include "GFX/CommandList.h"
#include "GFX/MeshData.h"

namespace ZE::RHI::DX12::Resource
{
	class Mesh final
	{
		D3D12_VERTEX_BUFFER_VIEW vertexView;
		D3D12_INDEX_BUFFER_VIEW indexView;
		bool is16bitIndices;
		ResourceInfo info;

		constexpr bool IsIndexBufferPresent() const noexcept { return vertexView.SizeInBytes != indexView.SizeInBytes; }
		constexpr U32 GetIndexSize() const noexcept { return is16bitIndices ? sizeof(U16) : sizeof(U32); }
		constexpr U32 GetIndexBytesOffset() const noexcept { return Math::AlignUp(vertexView.SizeInBytes, GetIndexSize()); }

	public:
		Mesh() = default;
		Mesh(GFX::Device& dev, const GFX::MeshData& data);
		ZE_CLASS_MOVE(Mesh);
		~Mesh() { ZE_ASSERT_FREED(info.IsFree()); }

		constexpr U32 GetVertexCount() const noexcept { return vertexView.SizeInBytes / vertexView.StrideInBytes; }
		constexpr U32 GetIndexCount() const noexcept { return IsIndexBufferPresent() ? (indexView.SizeInBytes - GetIndexBytesOffset()) / GetIndexSize() : 0; }
		void Free(GFX::Device& dev) noexcept { dev.Get().dx12.FreeBuffer(info); }

		void Draw(GFX::Device& dev, GFX::CommandList& cl) const noexcept(!_ZE_DEBUG_GFX_API);
		GFX::MeshData GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}
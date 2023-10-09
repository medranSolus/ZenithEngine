#pragma once
#include "GFX/Resource/MeshData.h"
#include "GFX/CommandList.h"
#include "IO/File.h"

namespace ZE::RHI::DX12::Resource
{
	class Mesh final
	{
		D3D12_VERTEX_BUFFER_VIEW vertexView;
		D3D12_INDEX_BUFFER_VIEW indexView;
		bool is16bitIndices;
		ResourceInfo info;

		constexpr bool IsIndexBufferPresent() const noexcept { return indexView.SizeInBytes != 0; }
		constexpr U32 GetIndexSize() const noexcept { return is16bitIndices ? sizeof(U16) : sizeof(U32); }

	public:
		Mesh() = default;
		Mesh(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::MeshData& data);
		Mesh(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::MeshFileData& data, IO::File& file);
		ZE_CLASS_MOVE(Mesh);
		~Mesh() { ZE_ASSERT_FREED(info.IsFree()); }

		constexpr U32 GetSize() const noexcept { return indexView.SizeInBytes + vertexView.SizeInBytes; }
		constexpr U32 GetVertexCount() const noexcept { return vertexView.SizeInBytes / vertexView.StrideInBytes; }
		constexpr U32 GetIndexCount() const noexcept { return indexView.SizeInBytes / GetIndexSize(); }
		constexpr U16 GetVertexSize() const noexcept { return Utils::SafeCast<U16>(vertexView.StrideInBytes); }
		constexpr PixelFormat GetIndexFormat() const noexcept { return is16bitIndices ? PixelFormat::R16_UInt : PixelFormat::R32_UInt; }
		void Free(GFX::Device& dev) noexcept { dev.Get().dx12.FreeBuffer(info); }

		void Draw(GFX::Device& dev, GFX::CommandList& cl) const noexcept(!_ZE_DEBUG_GFX_API);
		GFX::Resource::MeshData GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}
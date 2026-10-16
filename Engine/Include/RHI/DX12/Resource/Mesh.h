#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Resource/MeshData.h"
#include "GFX/CommandList.h"
#include "GFX/GFile.h"

namespace ZE::RHI::DX12::Resource
{
	class Mesh final
	{
		std::array<D3D12_VERTEX_BUFFER_VIEW, 2> vertexViews;
		D3D12_INDEX_BUFFER_VIEW indexView = {};
		GFX::Resource::MeshOptions options = Base(GFX::Resource::MeshOption::None);
		ResourceInfo info = {};

		constexpr bool IsSecondStreamPresent() const noexcept { return vertexViews.at(1).SizeInBytes != 0; }
		constexpr bool IsIndexBufferPresent() const noexcept { return indexView.SizeInBytes != 0; }
		constexpr bool Is16BitIndices() const noexcept { return (options & GFX::Resource::MeshOption::IndexFormat16bit) != 0; }
		constexpr U32 GetIndexOffset() const noexcept { return options & GFX::Resource::MeshOption::SourceDataFormatBBQ ? 2 * sizeof(Float4) : 0; }
		constexpr U32 GetIndexSize() const noexcept { return Is16BitIndices() ? sizeof(U16) : sizeof(U32); }

		Expected<U32> CreateMesh(Device& device, U32 vertexCount, U16 vertexSize, U16 vertexSizeSecondStream, U32 indexCount) noexcept;

	public:
		Mesh() = default;
		ZE_CLASS_MOVE(Mesh);
		~Mesh();

		static Expected<Mesh> Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::MeshData& data) noexcept;
		static Expected<Mesh> Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::MeshFileData& data, GFX::GFile& file) noexcept;

		constexpr U32 GetSize() const noexcept { return Math::AlignUp(indexView.SizeInBytes + GetIndexOffset(), GFX::Resource::MeshData::VERTEX_BUFFER_ALIGNMENT) + vertexViews.front().SizeInBytes + vertexViews.at(1).SizeInBytes; }
		constexpr U32 GetVertexCount() const noexcept { return vertexViews.front().SizeInBytes / vertexViews.front().StrideInBytes; }
		constexpr U32 GetIndexCount() const noexcept { return indexView.SizeInBytes / GetIndexSize(); }
		constexpr U16 GetVertexSize() const noexcept { return Utils::SafeCast<U16>(vertexViews.front().StrideInBytes + vertexViews.at(1).StrideInBytes); }
		constexpr PixelFormat GetIndexFormat() const noexcept { return Is16BitIndices() ? PixelFormat::R16_UInt : PixelFormat::R32_UInt; }

		void BindDecodeBuffer(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		void Draw(GFX::Device& dev, GFX::CommandList& cl) const noexcept;
	};
}
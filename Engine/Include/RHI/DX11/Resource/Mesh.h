#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Resource/MeshData.h"
#include "GFX/CommandList.h"
#include "GFX/GFile.h"
#include "CBuffer.h"

namespace ZE::RHI::DX11::Resource
{
	class Mesh final
	{
		DX::ComPtr<IBuffer> buffer;
		CBuffer decodeBuffer;
		std::array<U32, 2> vertexSizes;
		U32 vertexCount = 0;
		U32 indexCount = 0;
		GFX::Resource::MeshOptions options = Base(GFX::Resource::MeshOption::None);

		constexpr bool IsSecondStreamPresent() const noexcept { return vertexSizes.at(1) != 0; }
		constexpr bool IsIndexBufferPresent() const noexcept { return indexCount != 0; }
		constexpr bool Is16BitIndices() const noexcept { return (options & GFX::Resource::MeshOption::IndexFormat16bit) != 0; }
		constexpr U32 GetIndexOffset() const noexcept { return options & GFX::Resource::MeshOption::SourceDataFormatBBQ ? sizeof(Float4) * 2 : 0; }
		constexpr U32 GetIndexSize() const noexcept { return Is16BitIndices() ? sizeof(U16) : sizeof(U32); }

		Status CreateMesh(Device& device, D3D11_USAGE usage, D3D11_SUBRESOURCE_DATA* data, U32* size) noexcept;

	public:
		Mesh() = default;
		ZE_CLASS_MOVE(Mesh);
		~Mesh() = default;

		static Expected<Mesh> Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::MeshData& data) noexcept;
		static Expected<Mesh> Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::MeshFileData& data, GFX::GFile& file) noexcept;

		constexpr U32 GetSize() const noexcept { return Math::AlignUp(indexCount * GetIndexSize() + GetIndexOffset(), GFX::Resource::MeshData::VERTEX_BUFFER_ALIGNMENT) + vertexCount * GetVertexSize(); }
		constexpr U32 GetVertexCount() const noexcept { return vertexCount; }
		constexpr U32 GetIndexCount() const noexcept { return indexCount; }
		constexpr U16 GetVertexSize() const noexcept { return Utils::SafeCast<U16>(vertexSizes.front() + vertexSizes.at(1)); }
		constexpr PixelFormat GetIndexFormat() const noexcept { return Is16BitIndices() ? PixelFormat::R16_UInt : PixelFormat::R32_UInt; }

		void BindDecodeBuffer(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		void Draw(GFX::Device& dev, GFX::CommandList& cl) const noexcept;
	};
}
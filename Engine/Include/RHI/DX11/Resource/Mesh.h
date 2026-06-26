#pragma once
#include "GFX/Resource/MeshData.h"
#include "GFX/CommandList.h"
#include "GFX/GFile.h"

namespace ZE::RHI::DX11::Resource
{
	class Mesh final
	{
		DX::ComPtr<IBuffer> buffer;
		U32 vertexSize = 0;
		U32 vertexCount = 0;
		U32 indexCount = 0;
		bool is16bitIndices = false;

		constexpr bool IsIndexBufferPresent() const noexcept { return indexCount != 0; }
		constexpr U32 GetIndexSize() const noexcept { return is16bitIndices ? sizeof(U16) : sizeof(U32); }

	public:
		Mesh() = default;
		ZE_CLASS_MOVE(Mesh);
		~Mesh() = default;

		static Expected<Mesh> Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::MeshData& data) noexcept;
		static Expected<Mesh> Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::MeshFileData& data, GFX::GFile& file) noexcept;

		constexpr U32 GetSize() const noexcept { return indexCount * GetIndexSize() + vertexCount * vertexSize; }
		constexpr U32 GetVertexCount() const noexcept { return vertexCount; }
		constexpr U32 GetIndexCount() const noexcept { return indexCount; }
		constexpr U16 GetVertexSize() const noexcept { return Utils::SafeCast<U16>(vertexSize); }
		constexpr PixelFormat GetIndexFormat() const noexcept { return is16bitIndices ? PixelFormat::R16_UInt : PixelFormat::R32_UInt; }

		void Draw(GFX::Device& dev, GFX::CommandList& cl) const noexcept;
	};
}
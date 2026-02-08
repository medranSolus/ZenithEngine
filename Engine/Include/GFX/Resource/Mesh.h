#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/Resource/Mesh.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/Resource/Mesh.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/Resource/Mesh.h"
#endif

namespace ZE::GFX::Resource
{
	// Buffer holding indices into VertexBuffer
	class Mesh final
	{
		ZE_RHI_BACKEND(Resource::Mesh);

	public:
		Mesh() = default;
		ZE_CLASS_MOVE(Mesh);
		~Mesh() = default;

		static Expected<Mesh> Create(Device& dev, DiskManager& disk, const MeshData& data) noexcept;
		static Expected<Mesh> Create(Device& dev, DiskManager& disk, const MeshFileData& data, GFile& file) noexcept;
		ZE_RHI_BACKEND_GET(Resource::Mesh);

		// Main Gfx API

		constexpr U32 GetSize() const noexcept { ZE_RHI_BACKEND_CALL_RET(GetSize); }
		constexpr U32 GetVertexCount() const noexcept { ZE_RHI_BACKEND_CALL_RET(GetVertexCount); }
		constexpr U32 GetIndexCount() const noexcept { ZE_RHI_BACKEND_CALL_RET(GetIndexCount); }
		constexpr U16 GetVertexSize() const noexcept { ZE_RHI_BACKEND_CALL_RET(GetVertexSize); }
		constexpr PixelFormat GetIndexFormat() const noexcept { ZE_RHI_BACKEND_CALL_RET(GetIndexFormat); }

		constexpr void Draw(Device& dev, CommandList& cl) const noexcept { ZE_RHI_BACKEND_CALL(Draw, dev, cl); }
	};

#pragma region Functions
	Expected<Mesh> Mesh::Create(Device& dev, DiskManager& disk, const MeshData& data) noexcept
	{
		ZE_ASSERT(data.VertexCount && data.VertexSize, "Empty vertex data!");
		ZE_ASSERT(data.IndexCount == 0 || (data.IndexCount && data.IndexSize && data.IndexCount % 3 == 0),
			"Indices have to be multiple of 3!");
		ZE_RHI_BACKEND_CREATE(Resource::Mesh, dev, disk, data);
	}

	Expected<Mesh> Mesh::Create(Device& dev, DiskManager& disk, const MeshFileData& data, GFile& file) noexcept
	{
		ZE_ASSERT(data.SourceBytes && data.VertexCount && data.VertexSize, "Empty vertex data!");
		ZE_ASSERT(data.IndexCount % 3 == 0 && (data.IndexFormat == PixelFormat::Unknown
			|| data.IndexFormat == PixelFormat::R8_UInt || data.IndexFormat == PixelFormat::R16_UInt || data.IndexFormat == PixelFormat::R32_UInt),
			"Indices have to be multiple of 3 and one of the following formats: R8_UInt, R16_UInt or R32_UInt!");
		ZE_RHI_BACKEND_CREATE(Resource::Mesh, dev, disk, data, file);
	}
#pragma endregion
}
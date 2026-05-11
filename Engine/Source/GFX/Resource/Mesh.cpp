#include "GFX/Resource/Mesh.h"

namespace ZE::GFX::Resource
{
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
}
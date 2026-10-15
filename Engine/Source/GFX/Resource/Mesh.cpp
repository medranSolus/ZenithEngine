#include "GFX/Resource/Mesh.h"

namespace ZE::GFX::Resource
{
	Expected<Mesh> Mesh::Create(Device& dev, DiskManager& disk, const MeshData& data) noexcept
	{
		ZE_ASSERT(data.VertexCount && data.VertexSize, "Empty vertex data!");
		ZE_ASSERT(data.IndexCount % 3 == 0, "Indices have to be multiple of 3!");
		ZE_RHI_BACKEND_CREATE(Resource::Mesh, dev, disk, data);
	}

	Expected<Mesh> Mesh::Create(Device& dev, DiskManager& disk, const MeshFileData& data, GFile& file) noexcept
	{
		ZE_ASSERT(data.SourceBytes && data.VertexCount && data.VertexSize, "Empty vertex data!");
		ZE_ASSERT(data.IndexCount % 3 == 0, "Indices have to be multiple of 3!");
		ZE_RHI_BACKEND_CREATE(Resource::Mesh, dev, disk, data, file);
	}
}
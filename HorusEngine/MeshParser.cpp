#include "MeshParser.h"
#include "Visuals.h"

namespace FileService
{
	inline std::string MeshParser::GetMeshID(const aiMesh& mesh) noexcept
	{
		return ;
	}

	std::shared_ptr<GFX::Resource::IndexBuffer> MeshParser::GetIndexBuffer(GFX::Graphics& gfx, const aiMesh& mesh, const std::string& meshID) noexcept(!IS_DEBUG)
	{
		std::vector<unsigned int> indices;
		if (GFX::Resource::IndexBuffer::NotStored(meshID))
		{
			indices.reserve(static_cast<size_t>(mesh.mNumFaces) * 3);
			for (unsigned int i = 0; i < mesh.mNumFaces; ++i)
			{
				const auto& face = mesh.mFaces[i];
				assert(face.mNumIndices == 3);
				indices.emplace_back(face.mIndices[0]);
				indices.emplace_back(face.mIndices[1]);
				indices.emplace_back(face.mIndices[2]);
			}
		}
		return GFX::Resource::IndexBuffer::Get(gfx, meshID, std::move(indices));
	}

	std::shared_ptr<GFX::Resource::VertexBuffer> MeshParser::GetVertexBuffer(GFX::Graphics& gfx, const aiMesh& mesh,
		const std::string& meshID, std::shared_ptr<GFX::Data::VertexLayout> layout) noexcept(!IS_DEBUG)
	{
		if (GFX::Resource::VertexBuffer::NotStored(meshID))
			return GFX::Resource::VertexBuffer::Get(gfx, meshID, { layout, mesh });
		else
			return GFX::Resource::VertexBuffer::Get(gfx, meshID, { layout });
	}

	std::shared_ptr<GFX::Shape::Mesh> MeshParser::GetMesh(GFX::Graphics& gfx, const std::string& path, size_t index) const
	{
		const aiMesh& mesh = *meshes[index];
		std::string meshID = GetMeshID(mesh);
		auto indexBuffer = GetIndexBuffer(gfx, mesh, meshID);

		auto material = std::make_shared<GFX::Visual::Material>();

		std::shared_ptr<GFX::Data::VertexLayout> layout = std::make_shared<GFX::Data::VertexLayout>();
		layout->Append(VertexAttribute::Normal);

		GFX::Pipeline::Technique outlineTechnique("Outline");
		GFX::Pipeline::TechniqueStep maskStep(0);
		GFX::Pipeline::TechniqueStep outlineStep(0);



		meshID += layout->GetLayoutCode();

		auto vertexBuffer = GetVertexBuffer(gfx, mesh, meshID, layout);

		return std::shared_ptr<GFX::Shape::Mesh>();
	}
}
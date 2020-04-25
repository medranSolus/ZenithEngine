#pragma once
#include "Mesh.h"

namespace FileService
{
	class MeshParser
	{
		aiMesh* const* meshes = nullptr;
		aiMaterial* const* materials = nullptr;

		static inline std::string GetMeshID(const aiMesh& mesh) noexcept;

		static std::shared_ptr<GFX::Resource::IndexBuffer> GetIndexBuffer(GFX::Graphics& gfx,
			const aiMesh& mesh, const std::string& meshID) noexcept(!IS_DEBUG);
		static std::shared_ptr<GFX::Resource::VertexBuffer> GetVertexBuffer(GFX::Graphics& gfx,
			const aiMesh& mesh, const std::string& meshID, std::shared_ptr<GFX::Data::VertexLayout> layout) noexcept(!IS_DEBUG);

	public:
		constexpr MeshParser(aiMesh* const* meshes, aiMaterial* const* materials) noexcept : meshes(meshes), materials(materials) {}

		std::shared_ptr<GFX::Shape::Mesh> GetMesh(GFX::Graphics& gfx, const std::string& path, size_t index) const;
	};
}
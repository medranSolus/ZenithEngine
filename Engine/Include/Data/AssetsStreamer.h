#pragma once
#include "GFX/Resource/Texture/Schema.h"
#include "GFX/Resource/Mesh.h"
#include "Entity.h"
#if _ZE_EXTERNAL_MODEL_LOADING
ZE_WARNING_PUSH
#	include "assimp/Importer.hpp"
#	include "assimp/scene.h"
#	include "assimp/postprocess.h"
ZE_WARNING_POP
#endif

namespace ZE::Data
{
	// Manager performing dynamic load of data to resources and when to release them
	class AssetsStreamer final
	{
		Storage assets;
		GFX::Resource::Texture::Library texSchemaLib;

#if _ZE_EXTERNAL_MODEL_LOADING
		template<typename Index>
		static void ParseIndices(std::unique_ptr<Index[]>& indices, const aiMesh& mesh) noexcept;
		static void ParseNode(const aiNode& node, EID currentEntity, const std::vector<std::pair<EID, U32>>& meshes,
			const std::vector<EID>& materials, Storage& registry, bool loadNames) noexcept;

		EID ParseMesh(GFX::Device& dev, const aiMesh& mesh, bool loadNames);
		EID ParseMaterial(GFX::Device& dev, const aiMaterial& material,
			const GFX::Resource::Texture::Schema& texSchema, const std::string& path, bool loadNames);
#endif

	public:
		AssetsStreamer() = default;
		ZE_CLASS_MOVE(AssetsStreamer);
		~AssetsStreamer() = default;

		// TEMPORARY
		constexpr Storage& GetResources() noexcept { return assets; }
		constexpr GFX::Resource::Texture::Library& GetSchemaLibrary() noexcept { return texSchemaLib; }

#if _ZE_EXTERNAL_MODEL_LOADING
		void LoadModelData(GFX::Device& dev, Storage& registry, EID root, const std::string& file, bool loadNames = false);
#endif
	};
}
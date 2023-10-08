#pragma once
#include "MaterialPBR.h"
#include "IO/DiskManager.h"
#include "IO/FileStatus.h"
#include "IO/CompressionFormat.h"
#include "LOD.h"
#include "ResourceLocation.h"
#if _ZE_EXTERNAL_MODEL_LOADING
ZE_WARNING_PUSH
#	include "assimp/Importer.hpp"
#	include "assimp/scene.h"
#	include "assimp/postprocess.h"
ZE_WARNING_POP
#endif

namespace ZE::Data
{
	// Identifier of single geometry data
	struct MeshID { EID ID; };
	// Identifier of single material data
	struct MaterialID { EID ID; };

	// Management class for resources used by GPU
	class ResourceManager final
	{
	public:
		typedef U8 ResourceFlags;
		enum ResourceFlag : ResourceFlags { None = 0, Static = 1 };

		static constexpr const char* RESOURCE_FILE_EXT = ".zeres";

	private:
		struct PackID { U16 ID; };

		static constexpr const char* RESOURCE_DIR = "Resources";
		static constexpr const char* RESOURCE_FILE = "Resources/respack";

		IO::DiskManager diskManager;
		GFX::Resource::Texture::Library texSchemaLib;

#if _ZE_EXTERNAL_MODEL_LOADING
		template<typename Index>
		static void ParseIndices(std::unique_ptr<Index[]>& indices, const aiMesh& mesh) noexcept;
#endif

	public:
		ResourceManager() = default;
		ZE_CLASS_MOVE(ResourceManager);
		~ResourceManager() = default;

		constexpr IO::DiskManager& GetDisk() noexcept { return diskManager; }
		constexpr GFX::Resource::Texture::Library& GetSchemaLib() noexcept { return texSchemaLib; }

		Task<IO::FileStatus> LoadResourcePack(GFX::Device& dev, U16 packId) { return LoadResourcePack(dev, RESOURCE_FILE + std::to_string(packId) + RESOURCE_FILE_EXT); }
		Task<IO::FileStatus> SaveResourcePack(GFX::Device& dev, U16 packId, IO::CompressionFormat defaultCompression) { return SaveResourcePack(dev, RESOURCE_FILE + std::to_string(packId) + RESOURCE_FILE_EXT, packId, defaultCompression); }

		void Init(GFX::Device& dev);
		void Free(GFX::Device& dev);

		Task<IO::FileStatus> LoadResourcePack(GFX::Device& dev, std::string_view packFile);
		Task<IO::FileStatus> SaveResourcePack(GFX::Device& dev, std::string_view packFile, U16 packId, IO::CompressionFormat defaultCompression);
		void ShowWindow(GFX::Device& dev);

#if _ZE_EXTERNAL_MODEL_LOADING
		Task<MeshID> ParseMesh(GFX::Device& dev, const aiMesh& mesh);
		Task<MaterialID> ParseMaterial(GFX::Device& dev, const aiMaterial& material, const std::string& path);
#endif

		template<typename Material, typename MaterialFlags>
		MaterialID AddMaterial(GFX::Device& dev, MaterialFlags materialFlags, const Material& materialData,
			const GFX::Resource::Texture::PackDesc& textureDesc, ResourceFlags resourceFlags)
		{
			EID materialId = Settings::CreateEntity();
			Storage& data = Settings::Data;

			data.emplace<ResourceLocationAtom>(materialId, ResourceLocation::UploadingToGPU);
			data.emplace<Material>(materialId, materialData);
			data.emplace<MaterialFlags>(materialId, materialFlags);

			data.emplace<GFX::Material<Material, Material::TEX_SCHEMA_NAME>>(materialId, dev, materialData, textureDesc);

			return { materialId };
		}
	};
}
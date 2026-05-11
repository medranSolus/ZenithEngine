#pragma once
#include "GFX/DiskManager.h"
#include "IO/CompressionFormat.h"
#include "ExternalModelOptions.h"
#include "MaterialPBR.h"
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
	struct MeshID { EID ID = INVALID_EID; };
	// Identifier of single material data
	struct MaterialID { EID ID = INVALID_EID; };

	// Management class for performing dynamic loading of data to GPU
	class AssetsStreamer final
	{
	public:
		typedef U8 ResourceFlags;
		enum ResourceFlag : ResourceFlags { None = 0, Static = 1 };
		struct PackID { U16 ID = 0; };

		static constexpr const char* RESOURCE_FILE_EXT = ".zeres";

	private:
		struct DecompressionEntry
		{
			EID ResID = INVALID_EID;
			IO::CompressionFormat Format = IO::CompressionFormat::None;
			std::unique_ptr<U8[]> CompressedBuffer;
			U32 CompressedSize = 0;
		};

		static constexpr const char* RESOURCE_DIR = "Resources";
		static constexpr const char* RESOURCE_FILE = "Resources/respack";

		GFX::DiskManager diskManager;
		GFX::Resource::Texture::Library texSchemaLib;

#if _ZE_EXTERNAL_MODEL_LOADING
		template<typename Index>
		static void ParseIndices(Index* indices, const aiMesh& mesh) noexcept;
#endif

	public:
		AssetsStreamer() = default;
		ZE_CLASS_MOVE(AssetsStreamer);
		~AssetsStreamer() = default;

		static Expected<AssetsStreamer> Create(GFX::Device& dev) noexcept;

		constexpr GFX::DiskManager& GetDisk() noexcept { return diskManager; }
		constexpr GFX::Resource::Texture::Library& GetSchemaLib() noexcept { return texSchemaLib; }

		Task<Status> LoadResourcePack(GFX::Device& dev, U16 packId) noexcept { return LoadResourcePack(dev, RESOURCE_FILE + std::to_string(packId) + RESOURCE_FILE_EXT); }
		Task<Status> SaveResourcePack(GFX::Device& dev, U16 packId, IO::CompressionFormat defaultCompression) noexcept { return SaveResourcePack(dev, RESOURCE_FILE + std::to_string(packId) + RESOURCE_FILE_EXT, packId, defaultCompression); }

		Task<Status> LoadResourcePack(GFX::Device& dev, std::string_view packFile) noexcept;
		Task<Status> SaveResourcePack(GFX::Device& dev, std::string_view packFile, U16 packId, IO::CompressionFormat defaultCompression) noexcept;

#if _ZE_EXTERNAL_MODEL_LOADING
		Task<Expected<MeshID>> ParseMesh(GFX::Device& dev, const aiMesh& mesh) noexcept;
		Task<Expected<MaterialID>> ParseMaterial(GFX::Device& dev, const aiMaterial& material, const std::string& path, ExternalModelOptions options) noexcept;
#endif
		Status ShowWindow(GFX::Device& dev) noexcept;

		template<typename Material, typename MaterialFlags>
		Expected<MaterialID> AddMaterial(GFX::Device& dev, MaterialFlags materialFlags, const Material& materialData,
			const GFX::Resource::Texture::PackDesc& textureDesc, ResourceFlags resourceFlags) noexcept
		{
			EID materialId = Settings::CreateEntity();
			Settings::AssureEntityPools<Material, MaterialFlags, GFX::Material<Material, Material::TEX_SCHEMA_NAME>>();

			Settings::Data.emplace<Material>(materialId, materialData);
			Settings::Data.emplace<MaterialFlags>(materialId, materialFlags);

			auto exp = typename GFX::Material<Material, Material::TEX_SCHEMA_NAME>::Create(materialId, dev, materialData, textureDesc);
			if (!exp)
			{
				Settings::DestroyEntity(materialId);
				return std::unexpected(exp.error());
			}
			Settings::Data.emplace<GFX::Material<Material, Material::TEX_SCHEMA_NAME>>(std::move(*exp));

			return { materialId };
		}
	};
}
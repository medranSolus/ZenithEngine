#pragma once
#include "IO/CompressionFormat.h"
#include "IO/DiskManager.h"
#include "IO/FileStatus.h"
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
	struct MeshID { EID ID; };
	// Identifier of single material data
	struct MaterialID { EID ID; };

	// Management class for performing dynamic loading of data to GPU
	class AssetsStreamer final
	{
	public:
		typedef U8 ResourceFlags;
		enum ResourceFlag : ResourceFlags { None = 0, Static = 1 };
		struct PackID { U16 ID; };

		static constexpr const char* RESOURCE_FILE_EXT = ".zeres";

	private:
		struct DecompressionEntry
		{
			EID ResID;
			IO::CompressionFormat Format;
			std::unique_ptr<U8[]> CompressedBuffer;
			U32 CompressedSize;
		};

		static constexpr const char* RESOURCE_DIR = "Resources";
		static constexpr const char* RESOURCE_FILE = "Resources/respack";

		IO::DiskManager diskManager;
		GFX::Resource::Texture::Library texSchemaLib;

#if _ZE_EXTERNAL_MODEL_LOADING
		template<typename Index>
		static void ParseIndices(Index* indices, const aiMesh& mesh) noexcept;
#endif

	public:
		AssetsStreamer() = default;
		ZE_CLASS_MOVE(AssetsStreamer);
		~AssetsStreamer() = default;

		constexpr IO::DiskManager& GetDisk() noexcept { return diskManager; }
		constexpr GFX::Resource::Texture::Library& GetSchemaLib() noexcept { return texSchemaLib; }

		Task<IO::FileStatus> LoadResourcePack(GFX::Device& dev, U16 packId) { return LoadResourcePack(dev, RESOURCE_FILE + std::to_string(packId) + RESOURCE_FILE_EXT); }
		Task<IO::FileStatus> SaveResourcePack(GFX::Device& dev, U16 packId, IO::CompressionFormat defaultCompression) { return SaveResourcePack(dev, RESOURCE_FILE + std::to_string(packId) + RESOURCE_FILE_EXT, packId, defaultCompression); }

		void Init(GFX::Device& dev);
		void Free(GFX::Device& dev);

		Task<IO::FileStatus> LoadResourcePack(GFX::Device& dev, std::string_view packFile);
		Task<IO::FileStatus> SaveResourcePack(GFX::Device& dev, std::string_view packFile, U16 packId, IO::CompressionFormat defaultCompression);

#if _ZE_EXTERNAL_MODEL_LOADING
		Task<MeshID> ParseMesh(GFX::Device& dev, const aiMesh& mesh);
		Task<MaterialID> ParseMaterial(GFX::Device& dev, const aiMaterial& material, const std::string& path, ExternalModelOptions options);
#endif
		void ShowWindow(GFX::Device& dev);

		template<typename Material, typename MaterialFlags>
		MaterialID AddMaterial(GFX::Device& dev, MaterialFlags materialFlags, const Material& materialData,
			const GFX::Resource::Texture::PackDesc& textureDesc, ResourceFlags resourceFlags)
		{
			EID materialId = Settings::CreateEntity();
			Settings::AssureEntityPools<Material, MaterialFlags, GFX::Material<Material, Material::TEX_SCHEMA_NAME>>();

			Settings::Data.emplace<Material>(materialId, materialData);
			Settings::Data.emplace<MaterialFlags>(materialId, materialFlags);

			Settings::Data.emplace<GFX::Material<Material, Material::TEX_SCHEMA_NAME>>(materialId, dev, materialData, textureDesc);

			return { materialId };
		}
	};
}
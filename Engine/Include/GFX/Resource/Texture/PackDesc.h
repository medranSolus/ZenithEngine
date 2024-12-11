#pragma once
#include "GFX/Surface.h"
#include "IO/CompressionFormat.h"
#include "Schema.h"

namespace ZE::GFX::Resource::Texture
{
	// Info of how single texture should be created
	struct Desc
	{
		Type Type;
		std::vector<Surface> Surfaces;
	};

	// Info about single texture from file
	struct FileDesc
	{
		U32 Width = 0;
		U32 Height = 0;
		U16 DepthArraySize = 0;
		U16 MipLevels = 0;
		// To indicate empty texture entry, fill only Type field and set Format to Unknown
		PixelFormat Format = PixelFormat::Unknown;
		Type Type;
		U64 DataOffset = 0;
		U32 SourceBytes = 0;
		U32 UncompressedSize = 0;
		IO::CompressionFormat Compression = IO::CompressionFormat::None;
	};

	typedef U8 PackOptions;
	enum PackOption : PackOptions
	{
		// Textures are created as static pipeline resources during engine initializaion, currently not used
		StaticCreation = 1,
		// Textures will start with correct layout allowing for using them as copy sources only
		CopySource = 2,
	};

	// Describes set of textures to create pack with
	struct PackDesc
	{
		EID ResourceID = INVALID_EID;
		PackOptions Options = 0;
		std::vector<Desc> Textures;
#if _ZE_DEBUG_GFX_NAMES
		std::string DebugName = "";
#endif

		void Init(const Schema& schema) noexcept;
		void AddTexture(const Schema& schema, const std::string& name, std::vector<Surface>&& surfaces) noexcept;
		void AddTexture(Type type, std::vector<Surface>&& surfaces) noexcept;
	};

	// Set of textures description from file source
	struct PackFileDesc
	{
		EID ResourceID = INVALID_EID;
		PackOptions Options = 0;
		std::vector<FileDesc> Textures;

		void Init(const Schema& schema) noexcept;
		void AddTexture(U16 requestedlocation, const FileDesc& textureDesc) noexcept;
	};
}

#if _ZE_DEBUG_GFX_NAMES
// Sets name to be used as indentificator of created TexturePack resource
#	define ZE_TEXTURE_SET_NAME(texDesc, name) texDesc.DebugName = name
#else
// Sets name to be used as indentificator of created TexturePack resource
#	define ZE_TEXTURE_SET_NAME(texDesc, name)
#endif
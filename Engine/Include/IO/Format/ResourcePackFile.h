#pragma once
#include "GFX/Resource/Texture/PackDesc.h"
#include "IO/CompressionFormat.h"

namespace ZE::IO::Format
{
	/* File structure:
	*
	* ResourcePackFileHeader
	* ResourcePackEntry[]
	* ResourcePackTextureEntry[]
	* String names[]
	* Data[]
	*/

	typedef U16 ResourcePackFlags;
	// Flags describing resource pack file
	enum ResourcePackFlag : ResourcePackFlags { None = 0 };

	// Type of single entry in resource pack
	enum class ResourcePackEntryType : U8 { Geometry, Material, Buffer, Textures };

#pragma pack(push, 1)
	// Header of resource pack file
	struct ResourcePackFileHeader
	{
		static constexpr const char* SIGNATURE_STR = "ZERF";

		char Signature[4];
		U32 Version = 0;
		U32 ResourcesCount = 0;
		U32 TexturesCount = 0;
		U32 NameSectionSize = 0;
		U16 ID = 0;
		ResourcePackFlags Flags = 0;
	};

	// Single entry in resource pack info table with resource description
	struct ResourcePackEntry
	{
		ResourcePackEntryType Type;
		// Index from start of name section
		U32 NameIndex = 0;
		U16 NameSize = 0;
		union
		{
			struct
			{
				// Offset from start of file
				U64 Offset;
				U32 Bytes;
				U32 UncompressedSize;
				Float3 BoxCenter;
				Float3 BoxExtents;
				U32 VertexCount;
				U32 IndexCount;
				U16 VertexSize;
				PixelFormat IndexBufferFormat;
				CompressionFormat Compression;
			} Geometry;
			struct
			{
				// Offset from start of file
				U64 Offset;
				U32 Bytes;
				U32 UncompressedSize;
				U64 CustomFlags;
				CompressionFormat Compression;
			} Buffer;
			struct
			{
				// Index from start of ResourcePackTextureEntry section
				U32 TextureIndex;
				U16 TexturesCount;
				// Index from start of name section, only valid for known texture schemas and when not UINT32_MAX and size is not zero
				U32 SchemaNameIndex;
				U16 SchemaNameSize;
				GFX::Resource::Texture::PackOptions Options;
			} Textures;
		};
	};

	// Description of single texture in resource pack file
	struct ResourcePackTextureEntry
	{
		// Offset from start of file
		U64 Offset = 0;
		U32 Bytes = 0;
		U32 UncompressedSize = 0;
		U32 Width = 0;
		U32 Height = 0;
		U16 DepthArraySize = 0;
		U16 MipLevels = 0;
		PixelFormat Format = PixelFormat::Unknown;
		GFX::Resource::Texture::Type Type = GFX::Resource::Texture::Type::Tex1D;
		CompressionFormat Compression = CompressionFormat::None;
	};
#pragma pack(pop)
}
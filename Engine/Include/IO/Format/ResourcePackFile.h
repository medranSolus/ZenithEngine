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
		U32 Version;
		U32 ResourceCount;
		U32 TexturesCount;
		U32 NameSectionSize;
		U16 ID;
		ResourcePackFlags Flags;
	};

	// Single entry in resource pack info table with resource description
	struct ResourcePackEntry
	{
		ResourcePackEntryType Type;
		// Index from start of name section
		U32 NameIndex;
		U16 NameSize;
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
				U64 BufferOffset;
				U32 BufferSize;
				U32 UncompressedBufferSize;
				// Index from start of texture section
				U32 TextureIndex;
				U16 TexturesCount;
				GFX::Resource::Texture::PackOptions Options;
				CompressionFormat BufferCompression;
			} Material;
			struct
			{
				// Offset from start of file
				U64 Offset;
				U32 Bytes;
				U32 UncompressedSize;
				CompressionFormat Compression;
			} Buffer;
			struct
			{
				// Index from start of texture section
				U32 TextureIndex;
				U16 TexturesCount;
				GFX::Resource::Texture::PackOptions Options;
			} Textures;
		};
	};

	// Description of single texture in resource pack file
	struct ResourcePackTextureEntry
	{
		// Offset from start of file
		U64 Offset;
		U32 Bytes;
		U32 UncompressedSize;
		U32 Width;
		U32 Height;
		PixelFormat Format;
		GFX::Resource::Texture::Type Type;
		CompressionFormat Compression;
	};
#pragma pack(pop)
}
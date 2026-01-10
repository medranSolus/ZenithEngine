#pragma once
#include "PixelFormatDDS.h"

namespace ZE::IO::DDS
{
	// Flags used in DDS::Header
	typedef U32 HeaderFlags;
	// Possible flags describing contents of DDS::Header
	enum class HeaderFlag : HeaderFlags
	{
		// Required in every .dds file
		Caps = 1,
		// Required in every .dds file
		Height = 2,
		// Required in every .dds file
		Width = 4,
		// Required when pitch is provided for an uncompressed texture
		Pitch = 8,
		// Required in every .dds file
		PixelFormat = 0x1000,
		// Required in a mipmapped texture
		MipMapCount = 0x20000,
		// Required when pitch is provided for a compressed texture
		LinearSize = 0x80000,
		// Required in a depth texture
		Depth = 0x800000
	};
	ZE_ENUM_OPERATORS(HeaderFlag, HeaderFlags);

	// Description of complexity of DDS surface
	typedef U32 HeaderCaps;
	// Possible complexity values used in DDS::Header
	enum class HeaderCap : HeaderCaps
	{
		// Optional, must be used on any file that contains more than one surface
		// (a mipmap, a cubic environment map, or mipmapped volume texture) but not every writer uses this, so don't depend on it
		Complex = 8,
		// Optional, should be used for a mipmap
		MipMap = 0x400000,
		// Required on every texture but not every writer uses this, so don't depend on it
		Texture = 0x1000
	};
	ZE_ENUM_OPERATORS(HeaderCap, HeaderCaps);

	// Additional details about stored DDS surfaces
	typedef U32 HeaderCaps2;
	// Possible extended complexity details used in DDS::Header
	enum class HeaderCap2 : HeaderCaps2
	{
		// Required for a cube map
		Cubemap = 0x200,
		// Required when these surfaces are stored in a cube map
		CubemapPositiveX = 0x400,
		// Required when these surfaces are stored in a cube map
		CubemapNegativeX = 0x800,
		// Required when these surfaces are stored in a cube map
		CubemapPositiveY = 0x1000,
		// Required when these surfaces are stored in a cube map
		CubemapNegativeY = 0x2000,
		// Required when these surfaces are stored in a cube map
		CubemapPositiveZ = 0x4000,
		// Required when these surfaces are stored in a cube map
		CubemapNegativeZ = 0x8000,
		// Cubemaps require to have all 6 faces defined in a file
		CubemapAllFaces = CubemapPositiveX | CubemapNegativeX | CubemapPositiveY | CubemapNegativeY | CubemapPositiveZ | CubemapNegativeZ,
		// Required for a volume texture
		Volume = 0x200000
	};
	ZE_ENUM_OPERATORS(HeaderCap2, HeaderCaps2);

#pragma pack(push, 1)
	// Main header of DDS file format
	struct Header
	{
		// Structure size, must be 124
		U32 Size;
		HeaderFlags Flags;
		U32 Height;
		U32 Width;
		// The pitch or number of bytes per scan line in an uncompressed texture, the total number of bytes in the top level texture for a compressed texture
		U32 PitchOrLinearSize;
		// Depth of a volume texture (in pixels), otherwise unused
		U32 Depth;
		U32 MipMapCount;
		U32 Reserved1[11];
		PixelFormatDDS Format;
		HeaderCaps Caps;
		HeaderCaps2 Caps2;
		U32 Caps3;
		U32 Caps4;
		U32 Reserved2;
	};
#pragma pack(pop)

	static_assert(sizeof(Header) == 124, "Incorret size of DDS file header!");
}
#pragma once
#include "FormatDDS.h"
#include "Macros.h"

namespace ZE::DDS
{
#pragma pack(push, 1)
	// Identifies the type of resource in DDS file
	typedef U32 ResourceDimensions;
	// Possible resource formats in DDS::HeaderDXT10
	enum class ResourceDimension : ResourceDimensions { Texture1D = 2, Texture2D = 3, Texture3D = 4 };
	ZE_ENUM_OPERATORS(ResourceDimension, ResourceDimensions);

	// Identifies other, less common options for resources in DDS file
	typedef U32 MiscFlagsDXT10;
	// Possible less common resource formats in DDS::HeaderDXT10
	enum class MiscFlagDXT10 : MiscFlagsDXT10 { TextureCube };
	ZE_ENUM_OPERATORS(MiscFlagDXT10, MiscFlagsDXT10);

	// Contains additional metadata of DDS file
	typedef U32 MiscFlags2DXT10;
	// Possible additional info about DDS file from DDS::HeaderDXT10
	enum class MiscFlag2DXT10 : MiscFlags2DXT10
	{
		// Alpha channel content is unknown. This is the value for legacy files, which typically is assumed to be 'straight' alpha
		AlphaUnknown = 0,
		// Any alpha channel content is presumed to use straight alpha
		AlphaStraight = 1,
		// Any alpha channel content is using premultiplied alpha. The only legacy file formats that indicate this information are 'DX2' and 'DX4'
		AlphaPremultimied = 2,
		// Any alpha channel content is all set to fully opaque
		AlphaOpaque = 3,
		// Any alpha channel content is being used as a 4th channel and is not intended to represent transparency (straight or premultiplied)
		AlphaCustom = 4,
		// Mask to extract alpha mode
		AlphaModeMask = 0b0111
	};
	ZE_ENUM_OPERATORS(MiscFlag2DXT10, MiscFlags2DXT10);

	// DDS header extension to handle resource arrays
	struct HeaderDXT10
	{
		FormatDDS Format;
		ResourceDimensions Dimension;
		MiscFlagsDXT10 MiscFlag;
		// For a 2D texture that is also a cube-map texture, this number represents
		// the number of cubes so number of textures is 6 * ArraySize. For a 3D texture, this must be set to 1
		U32 ArraySize;
		MiscFlags2DXT10 MiscFlags2;
	};
#pragma pack(pop)

	static_assert(sizeof(HeaderDXT10) == 20, "Incorret size of DDS file DXT10 header!");
}
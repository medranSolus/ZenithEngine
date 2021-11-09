#pragma once
#include "Types.h"

namespace ZE
{
	// Supported pixel formats of images and textures
	enum class PixelFormat : U8
	{
		Unknown,
		// 4 x 32 bit component
		R32G32B32A32_Typeless,
		R32G32B32A32_Float,
		R32G32B32A32_UInt,
		R32G32B32A32_SInt,
		// 4 x 16 bit component
		R16G16B16A16_Typeless,
		R16G16B16A16_Float,
		R16G16B16A16_UInt,
		R16G16B16A16_SInt,
		R16G16B16A16_UNorm,
		R16G16B16A16_SNorm,
		// 4 x 8 bit component
		R8G8B8A8_Typeless,
		R8G8B8A8_UInt,
		R8G8B8A8_SInt,
		R8G8B8A8_UNorm,
		R8G8B8A8_UNorm_SRGB,
		R8G8B8A8_SNorm,
		B8G8R8A8_Typeless,
		B8G8R8A8_UNorm,
		B8G8R8A8_UNorm_SRGB,
		B8G8R8X8_Typeless, // Alpha unused
		B8G8R8X8_UNorm,
		B8G8R8X8_UNorm_SRGB,
		// 3 x 32 bit component
		R32G32B32_Typeless,
		R32G32B32_Float,
		R32G32B32_UInt,
		R32G32B32_SInt,
		// 2 x 32 bit component
		R32G32_Typeless,
		R32G32_Float,
		R32G32_UInt,
		R32G32_SInt,
		// 2 x 16 bit component
		R16G16_Typeless,
		R16G16_Float,
		R16G16_UInt,
		R16G16_SInt,
		R16G16_UNorm,
		R16G16_SNorm,
		// 2 x 8 bit component
		R8G8_Typeless,
		R8G8_UInt,
		R8G8_SInt,
		R8G8_UNorm,
		R8G8_SNorm,
		// 32 bit component
		R32_Typeless,
		R32_Float,
		R32_Depth,
		DepthOnly = 44, // Alias name for R32_Depth used in Depth Stencil
		R32_UInt,
		R32_SInt,
		// 16 bit component
		R16_Typeless,
		R16_Float,
		R16_UInt,
		R16_SInt,
		R16_UNorm,
		R16_SNorm,
		R16_Depth,
		DepthOnlySmall = 53, // Alias name for R32_Depth used in Depth Stencil
		// 8 bit component
		R8_Typeless,
		R8_UInt,
		R8_SInt,
		R8_UNorm,
		R8_SNorm,
		A8_UNorm, // Alpha only
		// Four-component block-compression
		BC1_Typeless,
		BC1_UNorm,
		BC1_UNorm_SRGB,
		// Four-component block-compression
		BC2_Typeless,
		BC2_UNorm,
		BC2_UNorm_SRGB,
		// Four-component block-compression
		BC3_Typeless,
		BC3_UNorm,
		BC3_UNorm_SRGB,
		// One-component block-compression
		BC4_Typeless,
		BC4_UNorm,
		BC4_SNorm,
		// Two-component block-compression
		BC5_Typeless,
		BC5_UNorm,
		BC5_SNorm,
		// Block-compression
		BC6H_Typeless,
		BC6H_UF16,
		BC6H_SF16,
		// Block-compression
		BC7_Typeless,
		BC7_UNorm,
		BC7_UNorm_SRGB,
		// Depth-stencil
		R24G8_Typeless,
		R24_UNorm_X8_Typeless,
		R24_Depth_S8_UInt,
		DepthStencil = 83, // Alias name for R24_Depth_S8_UInt used in Depth Stencil
		X24_Typeless_G8_UInt,
		// Depth only
		D32_Float_S8X24_UInt,
		R32_Float_X8X24_Typeless,
		X32_Typeless_G8X24_UInt,
		//Special
		R10G10B10A2_Typeless,
		R10G10B10A2_UInt,
		R10G10B10A2_UNorm,
		B4G4R4A4_UNorm,
		R8G8_B8G8_UNorm,
		G8R8_G8B8_UNorm,
		R10G10B10_XR_Bias_A2_UNorm,
		R9G9B9E5_SharedExp,
		R11G11B10_Float,
		B5G5R5A1_UNorm,
		B5G6R5_UNorm,
		R32G8X24_Typeless,
		R1_UNorm // Single bit
	};
}
#pragma once
#include "Types.h"

namespace ZE
{
	// Supported pixel formats of images and textures
	enum class PixelFormat : U8
	{
		Unknown,
		// 4 x 32 bit component
		R32G32B32A32_Float,
		R32G32B32A32_UInt,
		R32G32B32A32_SInt,
		// 4 x 16 bit component
		R16G16B16A16_Float,
		R16G16B16A16_UInt,
		R16G16B16A16_SInt,
		R16G16B16A16_UNorm,
		R16G16B16A16_SNorm,
		// 4 x 8 bit component
		R8G8B8A8_UInt,
		R8G8B8A8_SInt,
		R8G8B8A8_UNorm,
		R8G8B8A8_UNorm_SRGB,
		R8G8B8A8_SNorm,
		B8G8R8A8_UNorm,
		B8G8R8A8_UNorm_SRGB,
		// 3 x 32 bit component
		R32G32B32_Float,
		R32G32B32_UInt,
		R32G32B32_SInt,
		// 2 x 32 bit component
		R32G32_Float,
		R32G32_UInt,
		R32G32_SInt,
		// 2 x 16 bit component
		R16G16_Float,
		R16G16_UInt,
		R16G16_SInt,
		R16G16_UNorm,
		R16G16_SNorm,
		// 2 x 8 bit component
		R8G8_UInt,
		R8G8_SInt,
		R8G8_UNorm,
		R8G8_SNorm,
		// 32 bit component
		R32_Float,
		R32_Depth,
		DepthOnly = R32_Depth, // Alias name for R32_Depth used in Depth Stencil
		R32_UInt,
		R32_SInt,
		// 16 bit component
		R16_Float,
		R16_UInt,
		R16_SInt,
		R16_UNorm,
		R16_SNorm,
		R16_Depth,
		DepthOnly16 = R16_Depth, // Alias name for R16_Depth used in Depth Stencil
		// 8 bit component
		R8_UInt,
		R8_SInt,
		R8_UNorm,
		R8_SNorm,
		// Depth Stencil
		R24G8_DepthStencil,                // 24 bit + 8 bit
		R32G8_DepthStencil,                // 32 bit + 8 bit
		DepthStencil = R32G8_DepthStencil, // Alias name for R32G8_DepthStencil used in Depth Stencil
		// Compacted formats
		R10G10B10A2_UInt,     // 32 bit, 4 channels
		R10G10B10A2_UNorm,    // 32 bit, 4 channels
		R11G11B10_Float,      // 32 bit, 3 channels (15 biased exponent)
		R9G9B9E5_SharedExp,   // 32 bit, 3 channels (15 biased exponent, shared between channels)
		B4G4R4A4_UNorm,       // 16 bit, 4 channels
		B5G5R5A1_UNorm,       // 16 bit, 4 channels
		B5G6R5_UNorm,         // 16 bit, 3 channels
		// Four-component block-compression 1
		BC1_UNorm,
		BC1_UNorm_SRGB,
		// Four-component block-compression 2
		BC2_UNorm,
		BC2_UNorm_SRGB,
		// Four-component block-compression 3
		BC3_UNorm,
		BC3_UNorm_SRGB,
		// One-component block-compression 4
		BC4_UNorm,
		BC4_SNorm,
		// Two-component block-compression 5
		BC5_UNorm,
		BC5_SNorm,
		// Block-compression 6
		BC6H_UF16,
		BC6H_SF16,
		// Block-compression 7
		BC7_UNorm,
		BC7_UNorm_SRGB,
		// Video
		//YUV_Y416,       // YUV 4:4:4 format with 16 bits per channel
		YUV_Y410,       // YUV 4:4:4 format with 10 bits per channel
		//YUV_AYUV,       // YUV 4:4:4 format with 8 bits per channel
		YUV_Y216,       // YUV 4:2:2 format with 16 bits per channel
		YUV_Y210,       // YUV 4:2:2 format with 10 bits per channel
		YUV_YUY2,       // YUV 4:2:2 format with 8 bits per channel
		YUV_P208,       // Hybrid 4:2:2 format with 8 bits per channel
		YUV_P016,       // YUV 4:2:0 format with 16 bits per channel
		YUV_P010,       // YUV 4:2:0 format with 10 bits per channel
		YUV_NV12,       // YUV 4:2:0 format with 8 bits per channel
	};
}
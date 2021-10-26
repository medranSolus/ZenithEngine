#pragma once
#include "Topology.h"
#include <string>

namespace ZE::GFX::Resource
{
	// Logical blending modes
	enum class BlendType : U8 { None, Light, Normal };
	// Possible operations on stencil buffer
	enum class StencilMode : U8 { Off, Write, Mask, DepthOff, Reverse, DepthFirst };
	// GPU primitive culling
	enum class CullMode : U8 { None, Front, Back };

	// Creation descriptor for GPU state for draw calls
	struct PipelineStateDesc
	{
		std::wstring VS;
		std::wstring DS;
		std::wstring HS;
		std::wstring GS;
		std::wstring PS;
		BlendType Blender;
		StencilMode Stencil;
		CullMode Culling;
		bool DepthEnable;
		TopologyType Topology;
		U8 RenderTargetsCount;
		PixelFormat FormatsRT[8];
		PixelFormat FormatDS;
	};
}
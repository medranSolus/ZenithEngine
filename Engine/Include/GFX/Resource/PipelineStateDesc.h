#pragma once
#include "Topology.h"
#include <string>

namespace ZE::GFX::Resource
{
	enum class BlendType : U8 { None, Light, Normal };
	enum class StencilMode : U8 { Off, Write, Mask, DepthOff, Reverse, DepthFirst };
	enum class CullMode : U8 { None, Front, Back };

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
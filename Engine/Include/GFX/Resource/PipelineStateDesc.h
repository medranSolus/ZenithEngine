#pragma once
#include "Shader.h"
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
		Shader* VS;
		Shader* DS = nullptr;
		Shader* HS = nullptr;
		Shader* GS = nullptr;
		Shader* PS = nullptr;
		BlendType Blender = BlendType::None;
		StencilMode Stencil = StencilMode::Off;
		CullMode Culling = CullMode::Back;
		bool DepthClipEnable = true;
		TopologyType Topology;
		U8 RenderTargetsCount = 0;
		PixelFormat FormatsRT[8];
		PixelFormat FormatDS = PixelFormat::Unknown;

		static void SetShader(Shader*& shader, const wchar_t* name,
			std::unordered_map<std::wstring, Resource::Shader>& shaders) noexcept;
	};
}
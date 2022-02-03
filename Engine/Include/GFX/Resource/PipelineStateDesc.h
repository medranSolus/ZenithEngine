#pragma once
#include "Shader.h"
#include "Topology.h"
#include <bitset>

namespace ZE::GFX::Resource
{
	// Logical blending modes
	enum class BlendType : U8 { None, Light, Normal };
	// Possible operations on depth-stencil buffer
	enum class DepthStencilMode : U8 { StencilOff, StencilWrite, StencilMask, DepthOff, DepthReverse, DepthBefore };
	// GPU primitive culling
	enum class CullMode : U8 { None, Front, Back };

	// Creation descriptor for GPU state for draw calls
	struct PipelineStateDesc
	{
		// Vertex shader is always required
		Shader* VS;
		Shader* DS = nullptr;
		Shader* HS = nullptr;
		Shader* GS = nullptr;
		Shader* PS = nullptr;
		BlendType Blender = BlendType::None;
		DepthStencilMode DepthStencil = DepthStencilMode::StencilOff;
		CullMode Culling = CullMode::Back;
		TopologyType Topology = TopologyType::Triangle;
		TopologyOrder Ordering = TopologyOrder::List;
		U8 RenderTargetsCount = 0;
		PixelFormat FormatsRT[8];
		PixelFormat FormatDS = PixelFormat::Unknown;
		// Wireframe | DepthClip
		std::bitset<2> Flags = 1;
#if _ZE_MODE_DEBUG
		std::string DebugName = "Unknown";
#endif

		// Whether perform clipping based on depth value
		void SetDepthClip(bool val) noexcept { Flags[0] = val; }
		void SetWireFrame(bool val) noexcept { Flags[1] = val; }

		constexpr bool IsDepthClip() const noexcept { return Flags[0]; }
		constexpr bool IsWireFrame() const noexcept { return Flags[1]; }

		static void SetShader(Shader*& shader, const wchar_t* name,
			std::unordered_map<std::wstring, Resource::Shader>& shaders) noexcept;
	};
}

#if _ZE_MODE_DEBUG
// Sets name to be used as indentificator of created Pipeline State
#define ZE_PSO_SET_NAME(psoDesc, name) psoDesc.DebugName = name
#else
// Sets name to be used as indentificator of created Pipeline State
#define ZE_PSO_SET_NAME(psoDesc, name)
#endif
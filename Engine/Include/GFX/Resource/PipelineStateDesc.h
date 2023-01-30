#pragma once
#include "InputParam.h"
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
		Shader* VS = nullptr;
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
		std::vector<InputParam> InputLayout;
		// Wireframe | DepthClip | RelaxedRasterOrder | ConservativeRaser
		std::bitset<4> Flags = 1;
#if _ZE_DEBUG_GFX_NAMES
		std::string DebugName = "Unknown";
#endif

		// Whether perform clipping based on depth value
		void SetDepthClip(bool val) noexcept { Flags[0] = val; }
		void SetWireframe(bool val) noexcept { Flags[1] = val; }
		// Optimalization flag when:
		// - depth testing is enabled (modes: <, <=, >, >=) and primitives don't overlap in clip space
		// - primitives don't overlap in screen space
		// - depth testing is disabled and commutative blending is enabled (order of primitives doesn't affect the result)
		void SetRelaxedRasterOrder(bool val) noexcept { Flags[2] = val; }
		void SetConservativeRaster(bool val) noexcept { Flags[3] = val; }

		constexpr bool IsDepthClip() const noexcept { return Flags[0]; }
		constexpr bool IsWireframe() const noexcept { return Flags[1]; }
		constexpr bool IsRelaxedRasterOrder() const noexcept { return Flags[2]; }
		constexpr bool IsConservativeRaster() const noexcept { return Flags[3]; }

		static void SetShader(Device& dev, Shader*& shader, const char* name,
			std::unordered_map<std::string, Resource::Shader>& shaders) noexcept;
	};
}

#if _ZE_DEBUG_GFX_NAMES
// Sets name to be used as indentificator of created Pipeline State
#	define ZE_PSO_SET_NAME(psoDesc, name) psoDesc.DebugName = name
#else
// Sets name to be used as indentificator of created Pipeline State
#	define ZE_PSO_SET_NAME(psoDesc, name)
#endif
#pragma once
#include "DataPBR.h"
#include "ParamsPBR.h"
#include "RenderGraph.h"
#include "Info/World.h"

namespace ZE::GFX::Pipeline
{
	// Physically Based Renderer
	class RendererPBR : public RenderGraph
	{
		static constexpr U64 MESH_LIST_GROW_SIZE = 1024;
		static constexpr U64 DIR_LIGHT_LIST_GROW_SIZE = 24;
		static constexpr U64 SPOT_LIGHT_LIST_GROW_SIZE = 32;
		static constexpr U64 POINT_LIGHT_LIST_GROW_SIZE = 48;

		Info::World worldData;
		DataPBR settingsData;

		static void SetupRenderSlots(RendererBuildData& buildData) noexcept;

		constexpr void SetupBlurData(U32 width, U32 height, float sigma) noexcept;
		void SetupSsaoData(U32 width, U32 height) noexcept;

	public:
		RendererPBR() = default;
		ZE_CLASS_DELETE(RendererPBR);
		~RendererPBR();

		constexpr void SetActiveScene(const Data::Scene& scene) noexcept { worldData.ActiveScene = &scene; }
		constexpr U32 GetFrameWidth() const noexcept { return settingsData.FrameDimmensions.x; }
		constexpr U32 GetFrameHeight() const noexcept { return settingsData.FrameDimmensions.y; }

		void Init(Device& dev, CommandList& mainList, Resource::Texture::Library& texLib,
			U32 width, U32 height, const ParamsPBR& params);
		void UpdateWorldData(Device& dev, Data::EID camera) noexcept;
	};
}
#pragma once
#include "DataPBR.h"
#include "ParamsPBR.h"
#include "RenderGraph.h"
#include "WorldInfo.h"

namespace ZE::GFX::Pipeline
{
	// Physically Based Renderer
	class RendererPBR : public RenderGraph
	{
		static constexpr U64 MESH_LIST_GROW_SIZE = 64;

		WorldInfo worldData;
		DataPBR settingsData;

		static void SetupRenderSlots(RendererBuildData& buildData) noexcept;

		constexpr void SetupBlurData(U32 width, U32 height, float sigma) noexcept;
		void SetupSsaoData(U32 width, U32 height) noexcept;

	public:
		RendererPBR() = default;
		ZE_CLASS_DELETE(RendererPBR);
		~RendererPBR();

		constexpr void SetActiveScene(const Data::Scene& scene) noexcept { worldData.ActiveScene = &scene; }

		void Init(Device& dev, CommandList& mainList, Resource::Texture::Library& texLib,
			U32 width, U32 height, const ParamsPBR& params);
		void SetCurrentCamera(Device& dev, Data::EID camera);
		void UpdateWorldData() noexcept;
	};
}
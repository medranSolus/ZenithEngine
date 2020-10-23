#pragma once
#include "Technique.h"
#include "Visuals.h"

namespace GFX::Pipeline
{
	class TechniqueFactory
	{
		static Data::ColorFloat3 outlineColor;

		static std::shared_ptr<Technique> MakeLighting(RenderGraph& graph, const std::string& passName);

	public:
		TechniqueFactory() = delete;

		static inline std::shared_ptr<Technique> MakeDirectionalLighting(RenderGraph& graph) { return MakeLighting(graph, "dirLighting"); }
		static inline std::shared_ptr<Technique> MakeSpotLighting(RenderGraph& graph) { return MakeLighting(graph, "spotLighting"); }
		static inline std::shared_ptr<Technique> MakePointLighting(RenderGraph& graph) { return MakeLighting(graph, "pointLighting"); }

		static std::shared_ptr<Technique> MakeWireframe(RenderGraph& graph, std::shared_ptr<Visual::Material> material);
		static std::shared_ptr<Technique> MakeLambertian(Graphics& gfx, RenderGraph& graph, std::shared_ptr<Visual::Material> material);
		static std::shared_ptr<Technique> MakeShadowMap(Graphics& gfx, RenderGraph& graph, std::shared_ptr<Visual::Material> material);

		static std::shared_ptr<Technique> MakeOutlineBlur(Graphics& gfx, RenderGraph& graph,
			const std::string& name, std::shared_ptr<Data::VertexLayout> layout);

		static std::shared_ptr<Technique> MakeOutlineOffset(Graphics& gfx, RenderGraph& graph,
			const std::string& name, std::shared_ptr<Data::VertexLayout> layout);

		static std::shared_ptr<Technique> MakeOutlineScale(Graphics& gfx, RenderGraph& graph,
			const std::string& name, std::shared_ptr<Data::VertexLayout> layout);
	};
}
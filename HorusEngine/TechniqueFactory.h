#pragma once
#include "Technique.h"
#include "Visuals.h"

namespace GFX::Pipeline
{
	class TechniqueFactory
	{
		static Data::ColorFloat3 outlineColor;

		static Technique MakeLighting(RenderGraph& graph, const std::string& passName);

	public:
		TechniqueFactory() = delete;

		static inline Technique MakeDirectionalLighting(RenderGraph& graph) { return MakeLighting(graph, "dirLighting"); }
		static inline Technique MakeSpotLighting(RenderGraph& graph) { return MakeLighting(graph, "spotLighting"); }
		static inline Technique MakePointLighting(RenderGraph& graph) { return MakeLighting(graph, "pointLighting"); }

		static Technique MakeWireframe(RenderGraph& graph, std::shared_ptr<Visual::Material> material);
		static Technique MakeLambertian(Graphics& gfx, RenderGraph& graph, std::shared_ptr<Visual::Material> material);
		static Technique MakeShadowMap(Graphics& gfx, RenderGraph& graph, std::shared_ptr<Visual::Material> material);

		static Technique MakeOutlineBlur(Graphics& gfx, RenderGraph& graph,
			const std::string& name, std::shared_ptr<Data::VertexLayout> layout);

		static Technique MakeOutlineOffset(Graphics& gfx, RenderGraph& graph,
			const std::string& name, std::shared_ptr<Data::VertexLayout> layout);

		static Technique MakeOutlineScale(Graphics& gfx, RenderGraph& graph,
			const std::string& name, std::shared_ptr<Data::VertexLayout> layout);
	};
}
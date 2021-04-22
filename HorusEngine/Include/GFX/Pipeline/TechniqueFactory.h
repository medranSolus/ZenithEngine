#pragma once
#include "Technique.h"
#include "GFX/Visual/Visuals.h"

namespace GFX::Pipeline
{
	class TechniqueFactory
	{
		static inline ColorF3 outlineColor = { 1.0f, 1.0f, 0.0f };

		static Technique MakeLighting(RenderGraph& graph, std::string&& passName);

	public:
		TechniqueFactory() = delete;

		static Technique MakeDirectionalLighting(RenderGraph& graph) { return MakeLighting(graph, "dirLighting"); }
		static Technique MakeSpotLighting(RenderGraph& graph) { return MakeLighting(graph, "spotLighting"); }
		static Technique MakePointLighting(RenderGraph& graph) { return MakeLighting(graph, "pointLighting"); }

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
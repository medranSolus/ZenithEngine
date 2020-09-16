#pragma once
#include "Technique.h"
#include "Visuals.h"

namespace GFX::Pipeline
{
	class TechniqueFactory
	{
		static Data::ColorFloat3 outlineColor;

	public:
		TechniqueFactory() = delete;

		static std::shared_ptr<Technique> MakeWireframe(RenderGraph& graph, std::shared_ptr<Visual::Material> material);
		static std::shared_ptr<Technique> MakeLambertian(Graphics& gfx, RenderGraph& graph, std::shared_ptr<Visual::Material> material);
		static std::shared_ptr<Technique> MakeShadowMap(Graphics& gfx, RenderGraph& graph, std::shared_ptr<Data::VertexLayout> layout);

		static std::shared_ptr<Technique> MakeOutlineBlur(Graphics& gfx, RenderGraph& graph,
			const std::string& name, std::shared_ptr<Data::VertexLayout> layout);

		static std::shared_ptr<Technique> MakeOutlineOffset(Graphics& gfx, RenderGraph& graph,
			const std::string& name, std::shared_ptr<Data::VertexLayout> layout);

		static std::shared_ptr<Technique> MakeOutlineScale(Graphics& gfx, RenderGraph& graph,
			const std::string& name, std::shared_ptr<Data::VertexLayout> layout);
	};
}
#include "TechniqueFactory.h"

namespace GFX::Pipeline
{
	Data::ColorFloat3 TechniqueFactory::outlineColor = { 1.0f, 1.0f, 0.0f };

	std::shared_ptr<Technique> TechniqueFactory::MakeWireframe(RenderGraph& graph, std::shared_ptr<Visual::Material> material)
	{
		auto technique = std::make_shared<Pipeline::Technique>("Wireframe", RenderChannel::Main);
		technique->AddStep({ graph, "wireframe", material });
		return std::move(technique);
	}

	std::shared_ptr<Technique> TechniqueFactory::MakeLambertian(Graphics& gfx, RenderGraph& graph, std::shared_ptr<Visual::Material> material)
	{
		auto technique = std::make_shared<Pipeline::Technique>("Lambertian", RenderChannel::Main);
		technique->AddStep({ graph, "depthOnly", std::make_shared<Visual::DepthWrite>(gfx, material->GerVertexLayout()) });
		technique->AddStep({ graph, "lambertian", material });
		return std::move(technique);
	}

	std::shared_ptr<Technique> TechniqueFactory::MakeShadowMap(Graphics& gfx, RenderGraph& graph, std::shared_ptr<Data::VertexLayout> layout)
	{
		auto technique = std::make_shared<Pipeline::Technique>("Shadow Map", RenderChannel::Shadow);
		technique->AddStep({ graph, "lighting.shadowMap", std::make_shared<Visual::ShadowMap>(gfx, layout) });
		return std::move(technique);
	}

	std::shared_ptr<Technique> TechniqueFactory::MakeOutlineBlur(Graphics& gfx, RenderGraph& graph,
		const std::string& name, std::shared_ptr<Data::VertexLayout> layout)
	{
		auto technique = std::make_shared<Pipeline::Technique>("Blur Outline", RenderChannel::Main, false);
		technique->AddStep({ graph, "outlineGeneration", std::make_shared<Visual::DepthWrite>(gfx, layout) });
		technique->AddStep({ graph, "outlineDrawBlur", std::make_shared<Visual::OutlineMaskBlur>(gfx, name + "OutlineBlur", outlineColor, layout) });
		return std::move(technique);
	}

	std::shared_ptr<Technique> TechniqueFactory::MakeOutlineOffset(Graphics& gfx, RenderGraph& graph,
		const std::string& name, std::shared_ptr<Data::VertexLayout> layout)
	{
		auto technique = std::make_shared<Pipeline::Technique>("Offset Outline", RenderChannel::Main, false);
		technique->AddStep({ graph, "outlineGeneration", std::make_shared<Visual::DepthWrite>(gfx, layout) });
		technique->AddStep({ graph, "outlineDraw", std::make_shared<Visual::OutlineMaskOffset>(gfx, name + "OutlineOffset", outlineColor, layout) });
		return std::move(technique);
	}

	std::shared_ptr<Technique> TechniqueFactory::MakeOutlineScale(Graphics& gfx, RenderGraph& graph,
		const std::string& name, std::shared_ptr<Data::VertexLayout> layout)
	{
		auto technique = std::make_shared<Pipeline::Technique>("Scaling Outline", RenderChannel::Main, false);
		technique->AddStep({ graph, "outlineGeneration", std::make_shared<Visual::DepthWrite>(gfx, layout) });
		technique->AddStep({ graph, "outlineDraw", std::make_shared<Visual::OutlineMaskScale>(gfx, name + "OutlineScale", outlineColor, layout) });
		return std::move(technique);
	}
}
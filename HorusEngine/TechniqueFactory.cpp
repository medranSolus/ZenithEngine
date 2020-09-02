#include "TechniqueFactory.h"

namespace GFX::Pipeline
{
	Data::ColorFloat3 TechniqueFactory::outlineColor = { 1.0f, 1.0f, 0.0f };

	std::shared_ptr<Technique> TechniqueFactory::MakeLambertian(RenderGraph& graph, uint64_t channels, std::shared_ptr<Visual::Material> material) noexcept
	{
		auto technique = std::make_shared<Pipeline::Technique>("Lambertian", channels);
		technique->AddStep({ graph, "lambertian", material });
		return std::move(technique);
	}

	std::shared_ptr<Technique> TechniqueFactory::MakeWireframe(RenderGraph& graph, uint64_t channels, std::shared_ptr<Visual::Material> material) noexcept
	{
		auto technique = std::make_shared<Pipeline::Technique>("Wireframe", channels);
		technique->AddStep({ graph, "wireframe", material });
		return std::move(technique);
	}

	std::shared_ptr<Technique> TechniqueFactory::MakeOutlineBlur(Graphics& gfx, RenderGraph& graph, uint64_t channels,
		const std::string& name, std::shared_ptr<Data::VertexLayout> layout) noexcept
	{
		auto technique = std::make_shared<Pipeline::Technique>("Blur Outline", channels, false);
		technique->AddStep({ graph, "outlineGeneration", std::make_shared<Visual::OutlineWrite>(gfx, layout) });
		technique->AddStep({ graph, "outlineDrawBlur", std::make_shared<Visual::OutlineMaskBlur>(gfx, name + "OutlineBlur", outlineColor, layout) });
		return std::move(technique);
	}

	std::shared_ptr<Technique> TechniqueFactory::MakeOutlineOffset(Graphics& gfx, RenderGraph& graph, uint64_t channels,
		const std::string& name, std::shared_ptr<Data::VertexLayout> layout) noexcept
	{
		auto technique = std::make_shared<Pipeline::Technique>("Blur Outline", channels, false);
		technique->AddStep({ graph, "outlineGeneration", std::make_shared<Visual::OutlineWrite>(gfx, layout) });
		technique->AddStep({ graph, "outlineDraw", std::make_shared<Visual::OutlineMaskOffset>(gfx, name + "OutlineOffset", outlineColor, layout) });
		return std::move(technique);
	}

	std::shared_ptr<Technique> TechniqueFactory::MakeOutlineScale(Graphics& gfx, RenderGraph& graph, uint64_t channels,
		const std::string& name, std::shared_ptr<Data::VertexLayout> layout) noexcept
	{
		auto technique = std::make_shared<Pipeline::Technique>("Blur Outline", channels, false);
		technique->AddStep({ graph, "outlineGeneration", std::make_shared<Visual::OutlineWrite>(gfx, layout) });
		technique->AddStep({ graph, "outlineDraw", std::make_shared<Visual::OutlineMaskScale>(gfx, name + "OutlineScale", outlineColor, layout) });
		return std::move(technique);
	}
}
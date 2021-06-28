#include "GFX/Pipeline/TechniqueFactory.h"

namespace ZE::GFX::Pipeline
{
	Technique TechniqueFactory::MakeLighting(RenderGraph& graph, std::string&& passName)
	{
		Technique technique("Lighting", RenderChannel::Light);
		technique.AddStep({ graph, std::forward<std::string>(passName) });
		return technique;
	}

	Technique TechniqueFactory::MakeWireframe(RenderGraph& graph, std::shared_ptr<Visual::Material> material)
	{
		Technique technique("Wireframe", RenderChannel::Main);
		technique.AddStep({ graph, "wireframe", material });
		return technique;
	}

	Technique TechniqueFactory::MakeLambertian(Graphics& gfx, RenderGraph& graph, std::shared_ptr<Visual::Material> material)
	{
		Technique technique("Lambertian", RenderChannel::Main);
		if (material->IsTranslucent() || material->IsParallax())
			technique.AddStep({ graph, "lambertianClassic", material });
		else
		{
			material->SetDepthOnly(gfx);
			technique.AddStep({ graph, "lambertianDepthOptimized", material });
		}
		return technique;
	}

	Technique TechniqueFactory::MakeShadowMap(Graphics& gfx, RenderGraph& graph, std::shared_ptr<Visual::Material> material)
	{
		Technique technique("Shadow Map", RenderChannel::Shadow);
		auto shadowMap = std::make_shared<Visual::ShadowMap>(gfx, material);
		technique.AddStep({ graph, "dirLighting.shadowMap", shadowMap });
		technique.AddStep({ graph, "spotLighting.shadowMap", shadowMap });
		technique.AddStep({ graph, "pointLighting.shadowMap", std::make_shared<Visual::ShadowMapCube>(gfx, material) });
		return technique;
	}

	Technique TechniqueFactory::MakeOutlineBlur(Graphics& gfx, RenderGraph& graph,
		const std::string& name, std::shared_ptr<Data::VertexLayout> layout)
	{
		Technique technique("Blur Outline", RenderChannel::Main, false);
		technique.AddStep({ graph, "outlineGeneration", std::make_shared<Visual::DepthWrite>(gfx, layout) });
		technique.AddStep({ graph, "outlineDrawBlur", std::make_shared<Visual::OutlineMaskBlur>(gfx, name + "OutlineBlur", outlineColor, layout) });
		return technique;
	}

	Technique TechniqueFactory::MakeOutlineOffset(Graphics& gfx, RenderGraph& graph,
		const std::string& name, std::shared_ptr<Data::VertexLayout> layout)
	{
		Technique technique("Offset Outline", RenderChannel::Main, false);
		technique.AddStep({ graph, "outlineGeneration", std::make_shared<Visual::DepthWrite>(gfx, layout) });
		technique.AddStep({ graph, "outlineDraw", std::make_shared<Visual::OutlineMaskOffset>(gfx, name + "OutlineOffset", outlineColor, layout) });
		return technique;
	}

	Technique TechniqueFactory::MakeOutlineScale(Graphics& gfx, RenderGraph& graph,
		const std::string& name, std::shared_ptr<Data::VertexLayout> layout)
	{
		Technique technique("Scaling Outline", RenderChannel::Main, false);
		technique.AddStep({ graph, "outlineGeneration", std::make_shared<Visual::DepthWrite>(gfx, layout) });
		technique.AddStep({ graph, "outlineDraw", std::make_shared<Visual::OutlineMaskScale>(gfx, name + "OutlineScale", outlineColor, layout) });
		return technique;
	}
}
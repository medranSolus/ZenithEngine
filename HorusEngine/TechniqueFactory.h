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

		static std::shared_ptr<Technique> MakeLambertian(RenderGraph& graph, uint64_t channels, std::shared_ptr<Visual::Material> material) noexcept;
		static std::shared_ptr<Technique> MakeWireframe(RenderGraph& graph, uint64_t channels, std::shared_ptr<Visual::Material> material) noexcept;

		static std::shared_ptr<Technique> MakeOutlineBlur(Graphics& gfx, RenderGraph& graph, uint64_t channels,
			const std::string& name, std::shared_ptr<Data::VertexLayout> layout) noexcept;

		static std::shared_ptr<Technique> MakeOutlineOffset(Graphics& gfx, RenderGraph& graph, uint64_t channels,
			const std::string& name, std::shared_ptr<Data::VertexLayout> layout) noexcept;

		static std::shared_ptr<Technique> MakeOutlineScale(Graphics& gfx, RenderGraph& graph, uint64_t channels,
			const std::string& name, std::shared_ptr<Data::VertexLayout> layout) noexcept;
	};
}
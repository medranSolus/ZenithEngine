#pragma once
#include "RenderGraph.h"
#include "IVisual.h"

namespace GFX::Pipeline
{
	class TechniqueStep : public Probe::IProbeable
	{
		std::string targetPassName;
		RenderPass::Base::QueuePass* pass = nullptr;
		std::shared_ptr<Visual::IVisual> data = nullptr;

	public:
		inline TechniqueStep(const std::string& targetPass, std::shared_ptr<Visual::IVisual> data = nullptr) noexcept : targetPassName(targetPass), data(data) {}
		TechniqueStep(const TechniqueStep&) = default;
		TechniqueStep& operator=(const TechniqueStep&) = default;
		~TechniqueStep() = default;

		inline void AddData(std::shared_ptr<Visual::IVisual> stepData) noexcept { data = std::move(stepData); }
		inline void Submit(Shape::BaseShape& shape) noexcept { pass->Add({ &shape, this }); }
		inline void Bind(Graphics& gfx) noexcept { data->Bind(gfx); }
		inline void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { if (data) data->Accept(gfx, probe); }
		inline void SetParentReference(Graphics& gfx, const GfxObject& parent) { data->SetTransformBuffer(gfx, parent); }
		inline void Link(RenderGraph& graph) { assert(pass == nullptr); pass = &graph.GetRenderQueue(targetPassName); }
	};
}
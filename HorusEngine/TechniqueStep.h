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
		inline TechniqueStep(RenderGraph& graph, const std::string& targetPass, std::shared_ptr<Visual::IVisual> data = nullptr)
			: targetPassName(targetPass), data(data), pass(&graph.GetRenderQueue(targetPassName)) {}
		TechniqueStep(const TechniqueStep&) = default;
		TechniqueStep& operator=(const TechniqueStep&) = default;
		~TechniqueStep() = default;

		inline DirectX::XMFLOAT3 GetTransformPos() const noexcept { if (data) return data->GetTransformPos(); return { 0.0f,0.0f,0.0f }; }
		inline void AddData(std::shared_ptr<Visual::IVisual> stepData) noexcept { data = std::move(stepData); }
		inline void Submit(JobData& data) noexcept { pass->Add({ &data, this }); }
		inline void Bind(Graphics& gfx, RenderChannel mode = RenderChannel::All) { if (data) data->Bind(gfx, mode); }
		inline bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { if (data) return data->Accept(gfx, probe); return false; }
		inline void SetParentReference(Graphics& gfx, const GfxObject& parent) { if (data) data->SetTransformBuffer(gfx, parent); }
	};
}
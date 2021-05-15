#pragma once
#include "RenderGraph.h"
#include "GFX/Visual/IVisual.h"

namespace ZE::GFX::Pipeline
{
	class TechniqueStep : public Probe::IProbeable
	{
		mutable RenderPass::Base::QueuePass* pass = nullptr;
		std::shared_ptr<Visual::IVisual> data = nullptr;
		std::string targetPassName;

	public:
		TechniqueStep(RenderGraph& graph, std::string&& targetPass, std::shared_ptr<Visual::IVisual> data = nullptr)
			: data(data), pass(&graph.GetRenderQueue(targetPass)) { targetPassName = std::move(targetPass); }
		TechniqueStep(TechniqueStep&&) = default;
		TechniqueStep(const TechniqueStep&) = default;
		TechniqueStep& operator=(TechniqueStep&&) = default;
		TechniqueStep& operator=(const TechniqueStep&) = default;
		virtual ~TechniqueStep() = default;

		Float3 GetTransformPos() const noexcept { if (data) return data->GetTransformPos(); return { 0.0f,0.0f,0.0f }; }
		Matrix GetTransform() const noexcept { if (data) return data->GetTransform(); return Math::XMMatrixIdentity(); }

		void SetParentReference(Graphics& gfx, const GfxObject& parent) const { if (data) data->SetTransformBuffer(gfx, parent); }
		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { if (data) return data->Accept(gfx, probe); return false; }
		void Bind(Graphics& gfx, RenderChannel mode = RenderChannel::All) const { if (data) data->Bind(gfx, mode); }

		void Submit(const JobData& data) const noexcept;
	};
}
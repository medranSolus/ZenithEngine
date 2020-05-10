#pragma once
#include "RenderCommander.h"
#include "IVisual.h"

namespace GFX::Pipeline
{
	class TechniqueStep : public Probe::IProbeable
	{
		size_t targetPass;
		std::shared_ptr<Visual::IVisual> data = nullptr;

	public:
		inline TechniqueStep(size_t targetPass, std::shared_ptr<Visual::IVisual> data = nullptr) noexcept : targetPass(targetPass), data(data) {}
		TechniqueStep(const TechniqueStep&) = default;
		TechniqueStep& operator=(const TechniqueStep&) = default;
		~TechniqueStep() = default;

		inline void AddData(std::shared_ptr<Visual::IVisual> stepData) noexcept { data = std::move(stepData); }
		inline void Submit(RenderCommander& renderer, Shape::BaseShape& shape) noexcept { renderer.Add({ &shape, this }, targetPass); }
		inline void Bind(Graphics& gfx) noexcept { data->Bind(gfx); }
		inline void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { if (data) data->Accept(gfx, probe); }
	};
}
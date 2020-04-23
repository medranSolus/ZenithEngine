#pragma once
#include "TechniqueStep.h"

namespace GFX::Pipeline
{
	class Technique
	{
		bool active = true;
		std::vector<TechniqueStep> steps;

	public:
		constexpr bool IsActive() const noexcept { return active; }
		constexpr bool& IsActive() noexcept { return active; }
		constexpr void Activate() noexcept { active = true; }
		constexpr void Dectivate() noexcept { active = false; }
		inline void AddStep(TechniqueStep&& step) noexcept { steps.emplace_back(std::move(step)); }

		void Submit(RenderCommander& renderer, Shape::BaseShape& shape) noexcept;
	};
}
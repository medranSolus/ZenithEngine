#pragma once
#include "TechniqueStep.h"

namespace GFX::Pipeline
{
	class Technique
	{
		bool active;
		std::string name;
		std::vector<TechniqueStep> steps;

	public:
		inline Technique(const std::string& name, bool active = true) noexcept : active(active), name(name) {}
		inline Technique(std::string&& name, bool active = true) noexcept : active(active), name(std::move(name)) {}
		Technique(const Technique&) = default;
		Technique& operator=(const Technique&) = default;
		~Technique() = default;

		constexpr bool IsActive() const noexcept { return active; }
		constexpr bool& IsActive() noexcept { return active; }
		constexpr void Activate() noexcept { active = true; }
		constexpr void Dectivate() noexcept { active = false; }
		inline void AddStep(TechniqueStep&& step) noexcept { steps.emplace_back(std::move(step)); }

		void Submit(RenderCommander& renderer, Shape::BaseShape& shape) noexcept;
	};
}
#pragma once
#include "TechniqueStep.h"

namespace GFX::Pipeline
{
	class Technique : public Probe::IProbeable
	{
		bool active;
		std::string name;
		U64 channels;
		std::vector<TechniqueStep> steps;

	public:
		Technique(std::string&& name, U64 channels, bool active = true) noexcept
			: active(active), name(std::move(name)), channels(channels) {}
		Technique(Technique&&) = default;
		Technique(const Technique&) = default;
		Technique& operator=(Technique&&) = default;
		Technique& operator=(const Technique&) = default;
		virtual ~Technique() = default;

		constexpr const std::string& GetName() const noexcept { return name; }
		constexpr bool IsActive() const noexcept { return active; }
		constexpr bool& Active() noexcept { return active; }
		constexpr void Activate() noexcept { active = true; }
		constexpr void Deactivate() noexcept { active = false; }
		void AddStep(TechniqueStep&& step) noexcept { steps.emplace_back(std::forward<TechniqueStep>(step)); }

		void SetParentReference(Graphics& gfx, const GfxObject& parent) const;
		void Submit(const JobData& data, U64 channelFilter) const noexcept;
		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}
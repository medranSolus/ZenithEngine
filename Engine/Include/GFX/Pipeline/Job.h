#pragma once
#include "GFX/Graphics.h"
#include "RenderChannels.h"

namespace ZE::GFX::Pipeline
{
	class Job
	{
		const class JobData* data = nullptr;
		const class TechniqueStep* step = nullptr;

	public:
		constexpr Job(const class JobData* data, const class TechniqueStep* step) noexcept : data(data), step(step) {}
		Job(Job&&) = default;
		Job(const Job&) = default;
		Job& operator=(Job&&) = default;
		Job& operator=(const Job&) = default;
		~Job() = default;

		constexpr const class JobData& GetData() noexcept { return *data; }
		constexpr const class TechniqueStep& GetStep() const noexcept { return *step; }

		bool IsInsideFrustum(const Math::BoundingFrustum& volume) const noexcept;
		void Execute(Graphics& gfx, RenderChannel mode = RenderChannel::All) const;
	};
}
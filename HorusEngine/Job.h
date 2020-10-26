#pragma once
#include "Graphics.h"
#include "RenderChannels.h"

namespace GFX::Pipeline
{
	class Job
	{
		class JobData* data = nullptr;
		class TechniqueStep* step = nullptr;

	public:
		constexpr Job(class JobData* data, class TechniqueStep* step) noexcept : data(data), step(step) {}
		Job(const Job&) = default;
		Job& operator=(const Job&) = default;
		~Job() = default;

		constexpr class JobData& GetData() noexcept { return *data; }
		constexpr const class TechniqueStep& GetStep() const noexcept { return *step; }

		void Execute(Graphics& gfx, RenderChannel mode = RenderChannel::All);
	};
}
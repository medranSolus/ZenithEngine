#pragma once
#include "Job.h"

namespace GFX::Pipeline
{
	class RenderPass
	{
		std::vector<Job> jobs;

	public:
		RenderPass() = default;
		RenderPass(const RenderPass&) = default;
		RenderPass& operator=(const RenderPass&) = default;
		~RenderPass() = default;

		inline void Add(Job&& job) noexcept { jobs.emplace_back(std::move(job)); }
		inline void Reset() noexcept { jobs.clear(); }

		void Execute(Graphics& gfx) noexcept(!IS_DEBUG);
	};
}
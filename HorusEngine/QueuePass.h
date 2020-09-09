#pragma once
#include "BindingPass.h"
#include "Job.h"

namespace GFX::Pipeline::RenderPass::Base
{
	class QueuePass : public BindingPass
	{
		using BindingPass::BindingPass;

		std::vector<Job> jobs;

	public:
		virtual ~QueuePass() = default;

		inline void Add(Job&& job) noexcept { jobs.emplace_back(std::forward<Job>(job)); }
		inline void Reset() noexcept override { jobs.clear(); }

		void Execute(Graphics& gfx) override;
	};
}
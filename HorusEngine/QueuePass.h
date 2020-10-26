#pragma once
#include "BindingPass.h"
#include "Job.h"

namespace GFX::Pipeline::RenderPass::Base
{
	class QueuePass : public virtual BindingPass
	{
		using BindingPass::BindingPass;

		std::vector<Job> jobs;

		template<bool ascending>
		inline void Sort(const DirectX::XMFLOAT3& cameraPos) noexcept;

	protected:
		constexpr std::vector<Job>& GetJobs() noexcept { return jobs; }

		void SortFrontBack(const DirectX::XMFLOAT3& cameraPos) noexcept;
		void SortBackFront(const DirectX::XMFLOAT3& cameraPos) noexcept;

	public:
		virtual ~QueuePass() = default;

		inline void Add(Job&& job) noexcept { jobs.emplace_back(std::forward<Job>(job)); }
		inline void Reset() noexcept override { jobs.clear(); }
		inline void Execute(Graphics& gfx) override { Execute(gfx, RenderChannel::All); }

		void Execute(Graphics& gfx, RenderChannel mode);
	};
}
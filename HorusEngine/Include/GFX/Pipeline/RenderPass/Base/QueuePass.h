#pragma once
#include "BindingPass.h"
#include "GFX/Pipeline/Job.h"
#include "Camera/ICamera.h"

namespace GFX::Pipeline::RenderPass::Base
{
	class QueuePass : public virtual BindingPass
	{
		std::vector<Job> jobs;

		template<bool Ascending>
		void Sort(const Float3& cameraPos) noexcept;

	protected:
		constexpr std::vector<Job>& GetJobs() noexcept { return jobs; }

		void SortFrontBack(const Float3& cameraPos) noexcept;
		void SortBackFront(const Float3& cameraPos) noexcept;
		void CullFrustum(const Camera::ICamera& camera) noexcept;

		using BindingPass::BindingPass;

	public:
		QueuePass(QueuePass&&) = default;
		QueuePass(const QueuePass&) = default;
		QueuePass& operator=(QueuePass&&) = default;
		QueuePass& operator=(const QueuePass&) = default;
		virtual ~QueuePass() = default;

		void Add(Job&& job) noexcept { jobs.emplace_back(std::forward<Job>(job)); }
		void Reset() noexcept override { jobs.clear(); }
		void Execute(Graphics& gfx) override { Execute(gfx, RenderChannel::All); }

		void Execute(Graphics& gfx, RenderChannel mode);
	};
}
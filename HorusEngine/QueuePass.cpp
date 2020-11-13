#include "QueuePass.h"
#include "JobData.h"

namespace GFX::Pipeline::RenderPass::Base
{
	template<bool ascending>
	inline void QueuePass::Sort(const DirectX::XMFLOAT3& cameraPos) noexcept
	{
		const DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&cameraPos);
		std::sort(GetJobs().begin(), GetJobs().end(), [&pos](const Job& j1, const Job& j2)
			{
				const float len1 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&j1.GetStep().GetTransformPos()), pos)));
				const float len2 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&j2.GetStep().GetTransformPos()), pos)));
				if constexpr (ascending)
					return len1 < len2;
				else
					return len1 > len2;
			});
	}

	void QueuePass::SortFrontBack(const DirectX::XMFLOAT3& cameraPos) noexcept
	{
		Sort<true>(cameraPos);
	}

	void QueuePass::SortBackFront(const DirectX::XMFLOAT3& cameraPos) noexcept
	{
		Sort<false>(cameraPos);
	}

	void QueuePass::CullFrustum(const Camera::ICamera& camera) noexcept
	{
		const DirectX::BoundingFrustum volume = camera.GetFrustum();
		size_t rejectedIndex = 0, rejectedCount = 0;
		for (size_t i = 0, size = jobs.size(); i < size; ++i)
		{
			if (!jobs.at(i).IsInsideFrustum(volume))
				++rejectedCount;
			else
			{
				if (rejectedCount)
				{
					jobs.erase(jobs.begin() + rejectedIndex, jobs.begin() + rejectedIndex + rejectedCount);
					i -= rejectedCount;
					rejectedIndex = i;
					rejectedCount = 0;
					size = jobs.size();
				}
				++rejectedIndex;
			}
		}
		if (rejectedCount)
			jobs.erase(jobs.begin() + rejectedIndex, jobs.begin() + rejectedIndex + rejectedCount);
	}

	void QueuePass::Execute(Graphics& gfx, RenderChannel mode)
	{
		DRAW_TAG_START(gfx, GetName());
		BindAll(gfx);
		for (auto& job : jobs)
		{
			DRAW_TAG_START(gfx, job.GetData().GetName());
			job.Execute(gfx, mode);
			DRAW_TAG_END(gfx);
		}
		DRAW_TAG_END(gfx);
	}
}
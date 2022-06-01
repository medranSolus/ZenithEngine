#include "GFX/Pipeline/RenderPass/Base/QueuePass.h"
#include "GFX/Pipeline/JobData.h"

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	template<bool Ascending>
	void QueuePass::Sort(const Float3& cameraPos) noexcept
	{
		const Vector cpos = Math::XMLoadFloat3(&cameraPos);
		std::sort(GetJobs().begin(), GetJobs().end(), [&cpos](const Job& j1, const Job& j2)
			{
				const auto pos1 = j1.GetStep().GetTransformPos();
				const auto pos2 = j2.GetStep().GetTransformPos();
				const float len1 = Math::XMVectorGetX(Math::XMVector3Length(Math::XMVectorSubtract(Math::XMLoadFloat3(&pos1), cpos)));
				const float len2 = Math::XMVectorGetX(Math::XMVector3Length(Math::XMVectorSubtract(Math::XMLoadFloat3(&pos2), cpos)));
				if constexpr (Ascending)
					return len1 < len2;
				else
					return len1 > len2;
			});
	}

	void QueuePass::SortFrontBack(const Float3& cameraPos) noexcept
	{
		Sort<true>(cameraPos);
	}

	void QueuePass::SortBackFront(const Float3& cameraPos) noexcept
	{
		Sort<false>(cameraPos);
	}

	void QueuePass::CullFrustum(const Camera::ICamera& camera) noexcept
	{
		const Math::BoundingFrustum volume = camera.GetFrustum();
		U64 rejectedIndex = 0, rejectedCount = 0;
		for (U64 i = 0, size = jobs.size(); i < size; ++i)
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

	void QueuePass::Execute(Graphics& gfx, RenderChannel mode, U8 type, U64 lightNumber)
	{
		ZE_DRAW_TAG_START(gfx, GetName());
		BindAll(gfx);
		for (auto& job : jobs)
		{
			ZE_DRAW_TAG_START(gfx, job.GetData().GetName());
			job.Execute(gfx, mode, type, lightNumber);
			ZE_DRAW_TAG_END(gfx);
		}
		ZE_DRAW_TAG_END(gfx);
	}
}
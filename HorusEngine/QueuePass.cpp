#include "QueuePass.h"
#include "TechniqueStep.h"

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

	void QueuePass::Execute(Graphics& gfx, RenderChannel mode)
	{
		BindAll(gfx);
		for (auto& job : jobs)
			job.Execute(gfx, mode);
	}
}
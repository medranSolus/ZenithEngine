#include "TechniqueStep.h"
#include "JobData.h"

namespace GFX::Pipeline
{
	bool Job::IsInsideFrustum(const DirectX::BoundingFrustum& volume) const noexcept
	{
		return data->GetBoundingBox().Intersects(volume, step->GetTransform());
	}

	void Job::Execute(Graphics& gfx, RenderChannel mode)
	{
		data->Bind(gfx);
		step->Bind(gfx, mode);
		gfx.DrawIndexed(data->GetIndexCount());
	}
}
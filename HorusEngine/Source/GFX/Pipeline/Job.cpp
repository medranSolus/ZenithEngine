#include "GFX/Pipeline/Job.h"
#include "GFX/Pipeline/JobData.h"

namespace GFX::Pipeline
{
	bool Job::IsInsideFrustum(const Math::BoundingFrustum& volume) const noexcept
	{
		return data->GetBoundingBox().Intersects(volume, step->GetTransform());
	}

	void Job::Execute(Graphics& gfx, RenderChannel mode) const
	{
		data->Bind(gfx);
		step->Bind(gfx, mode);
		gfx.DrawIndexed(data->GetIndexCount());
	}
}
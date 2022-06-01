#include "GFX/Pipeline/Job.h"
#include "GFX/Pipeline/JobData.h"

namespace ZE::GFX::Pipeline
{
	bool Job::IsInsideFrustum(const Math::BoundingFrustum& volume) const noexcept
	{
		return data->GetBoundingBox().Intersects(volume, step->GetTransform());
	}

	void Job::Execute(Graphics& gfx, RenderChannel mode, U8 type, U64 lightNumber) const
	{
		data->Bind(gfx);
		step->Bind(gfx, mode);
		switch (type)
		{
		case 1:
			ZE_PERF_START("Lambertian Draw");
			break;
		case 2:
			ZE_PERF_START("Point Light Draw");
			break;
		case 3:
			ZE_PERF_START("Spot Light Draw");
			break;
		case 4:
			ZE_PERF_START("Point Light " + std::to_string(lightNumber) + " - Shadow Map Draw");
			break;
		case 5:
			ZE_PERF_START("Spot Light " + std::to_string(lightNumber) + " - Shadow Map Draw");
			break;
		default:
			break;
		}
		gfx.DrawIndexed(data->GetIndexCount());
		switch (type)
		{
		default:
			ZE_PERF_STOP();
		case 0:
			break;
		}
	}
}
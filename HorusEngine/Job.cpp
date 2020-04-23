#include "Job.h"
#include "BaseShape.h"
#include "TechniqueStep.h"

namespace GFX::Pipeline
{
	void Job::Execute(Graphics& gfx) noexcept(!IS_DEBUG)
	{
		shape->Bind(gfx);
		step->Bind(gfx);
		gfx.DrawIndexed(shape->GetIndexCount());
	}
}
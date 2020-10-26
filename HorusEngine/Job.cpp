#include "TechniqueStep.h"
#include "JobData.h"

namespace GFX::Pipeline
{
	void Job::Execute(Graphics& gfx, RenderChannel mode)
	{
		data->Bind(gfx);
		step->Bind(gfx, mode);
		gfx.DrawIndexed(data->GetIndexCount());
	}
}
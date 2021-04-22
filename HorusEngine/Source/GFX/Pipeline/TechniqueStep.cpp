#include "GFX/Pipeline/TechniqueStep.h"
#include "GFX/Pipeline/JobData.h"

namespace GFX::Pipeline
{
	void TechniqueStep::Submit(const JobData& data) const noexcept
	{
		Job job(&data, this);
		pass->Add(std::move(job));
	}
}
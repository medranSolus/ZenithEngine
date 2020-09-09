#include "QueuePass.h"

namespace GFX::Pipeline::RenderPass::Base
{
	void QueuePass::Execute(Graphics& gfx)
	{
		BindAll(gfx);
		for (auto& job : jobs)
			job.Execute(gfx);
	}
}
#include "QueuePass.h"

namespace GFX::Pipeline::RenderPass::Base
{
	void QueuePass::Execute(Graphics& gfx, RenderChannel mode)
	{
		BindAll(gfx);
		for (auto& job : jobs)
			job.Execute(gfx, mode);
	}
}
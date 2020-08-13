#include "QueuePass.h"

namespace GFX::Pipeline::RenderPass::Base
{
	void QueuePass::Execute(Graphics& gfx) noexcept(!IS_DEBUG)
	{
		BindAll(gfx);
		for (auto& job : jobs)
			job.Execute(gfx);
	}
}
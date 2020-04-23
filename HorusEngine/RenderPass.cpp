#include "RenderPass.h"

namespace GFX::Pipeline
{
	void RenderPass::Execute(Graphics& gfx) noexcept(!IS_DEBUG)
	{
		for (auto& job : jobs)
			job.Execute(gfx);
	}
}
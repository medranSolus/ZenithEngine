#include "RenderCommander.h"

namespace GFX::Pipeline
{
	void RenderCommander::Render(Graphics& gfx) noexcept(!IS_DEBUG)
	{
		// Shenenigans
	}

	void RenderCommander::Reset() noexcept
	{
		for (auto& pass : passes)
			pass.Reset();
	}
}
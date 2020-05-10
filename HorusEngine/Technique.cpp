#include "Technique.h"

namespace GFX::Pipeline
{
	void Technique::Submit(RenderCommander& renderer, Shape::BaseShape& shape) noexcept
	{
		if (active)
		{
			for (auto& step : steps)
				step.Submit(renderer, shape);
		}
	}

	void Technique::Accept(Probe& probe) noexcept
	{
		probe.SetTechnique(this);
		if (active)
			for (auto& step : steps)
				step.Accept(probe);
		probe.Release();
	}
}
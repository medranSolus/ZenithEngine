#include "Technique.h"

namespace GFX::Pipeline
{
	void Technique::SetParentReference(Graphics& gfx, const GfxObject& parent)
	{
		for (auto& step : steps)
			step.SetParentReference(gfx, parent);
	}

	void Technique::Submit(Shape::BaseShape& shape) noexcept
	{
		if (active)
			for (auto& step : steps)
				step.Submit(shape);
	}

	void Technique::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		probe.SetTechnique(this);
		if (active)
			for (auto& step : steps)
				step.Accept(gfx, probe);
		probe.ReleaseTechnique();
	}
}
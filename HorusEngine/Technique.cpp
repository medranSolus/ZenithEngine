#include "Technique.h"

namespace GFX::Pipeline
{
	void Technique::SetParentReference(Graphics& gfx, const GfxObject& parent)
	{
		for (auto& step : steps)
			step.SetParentReference(gfx, parent);
	}

	void Technique::Submit(JobData& data, uint64_t channelFilter) noexcept
	{
		if (active && channels & channelFilter)
			for (auto& step : steps)
				step.Submit(data);
	}

	bool Technique::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		bool change = false;
		probe.SetTechnique(this);
		if (active)
			for (auto& step : steps)
				change |= step.Accept(gfx, probe);
		probe.ReleaseTechnique();
		return change;
	}
}
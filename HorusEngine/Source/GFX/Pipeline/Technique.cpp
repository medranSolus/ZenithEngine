#include "GFX/Pipeline/Technique.h"

namespace GFX::Pipeline
{
	void Technique::SetParentReference(Graphics& gfx, const GfxObject& parent) const
	{
		for (auto& step : steps)
			step.SetParentReference(gfx, parent);
	}

	void Technique::Submit(const JobData& data, U64 channelFilter) const noexcept
	{
		if (active && channels & channelFilter)
			for (auto& step : steps)
				step.Submit(data);
	}

	bool Technique::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		bool change = false;
		if (active)
		{
			probe.SetTechnique(this);
			for (auto& step : steps)
				change |= step.Accept(gfx, probe);
			probe.ReleaseTechnique();
		}
		return change;
	}
}
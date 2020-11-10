#include "JobData.h"

namespace GFX::Pipeline
{
	void JobData::SetTechniques(Graphics& gfx, std::vector<Technique>&& newTechniques, const GfxObject& parent) noexcept
	{
		techniques = std::move(newTechniques);
		for (auto& technique : techniques)
			technique.SetParentReference(gfx, parent);
	}

	Technique* JobData::GetTechnique(const std::string& name) noexcept
	{
		for (auto& technique : techniques)
			if (technique.GetName().find(name) != std::string::npos)
				return &technique;
		return nullptr;
	}

	void JobData::Submit(uint64_t channelFilter) noexcept
	{
		for (auto& technique : techniques)
			technique.Submit(*this, channelFilter);
	}

	bool JobData::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		bool change = false;
		for (auto& technique : techniques)
			change |= technique.Accept(gfx, probe);
		return change;
	}
}
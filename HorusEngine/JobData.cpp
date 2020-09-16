#include "JobData.h"

namespace GFX::Pipeline
{
	void JobData::SetTechniques(Graphics& gfx, std::vector<std::shared_ptr<Pipeline::Technique>>&& newTechniques, const GfxObject& parent) noexcept
	{
		techniques = std::move(newTechniques);
		for (auto& technique : techniques)
			technique->SetParentReference(gfx, parent);
	}

	void JobData::Submit(uint64_t channelFilter) noexcept
	{
		for (auto& technique : techniques)
			technique->Submit(*this, channelFilter);
	}

	void JobData::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		for (auto& technique : techniques)
			technique->Accept(gfx, probe);
	}
}
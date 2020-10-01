#pragma once
#include "IRenderable.h"
#include "Technique.h"

namespace GFX::Pipeline
{
	class JobData : public virtual IRenderable
	{
	protected:
		std::vector<std::shared_ptr<Pipeline::Technique>> techniques;

		void SetTechniques(Graphics& gfx, std::vector<std::shared_ptr<Pipeline::Technique>>&& newTechniques, const GfxObject& parent) noexcept;

	public:
		JobData() = default;
		inline JobData(JobData&& data) noexcept { *this = std::forward<JobData&&>(data); }
		inline JobData& operator=(JobData&& data) noexcept { techniques = std::move(data.techniques); return *this; }
		virtual ~JobData() = default;

		virtual UINT GetIndexCount() const noexcept = 0;
		virtual void Bind(Graphics& gfx) = 0;

		void Submit(uint64_t channelFilter) noexcept override;
		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}
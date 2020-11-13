#pragma once
#include "IRenderable.h"
#include "Technique.h"
#include "BoundingBox.h"

namespace GFX::Pipeline
{
	class JobData : public virtual IRenderable
	{
	protected:
		std::vector<Technique> techniques;

		inline void AddTechnique(Graphics& gfx, Technique&& technique) noexcept { techniques.emplace_back(std::forward<Technique&&>(technique)); }

		void SetTechniques(Graphics& gfx, std::vector<Technique>&& newTechniques, const GfxObject& parent) noexcept;

	public:
		JobData() = default;
		inline JobData(JobData&& data) noexcept { *this = std::forward<JobData&&>(data); }
		inline JobData& operator=(JobData&& data) noexcept { techniques = std::move(data.techniques); return *this; }
		virtual ~JobData() = default;

		virtual const std::string& GetName() const noexcept = 0;
		virtual const Data::BoundingBox& GetBoundingBox() const noexcept = 0;
		virtual UINT GetIndexCount() const noexcept = 0;
		virtual void Bind(Graphics& gfx) = 0;

		Technique* GetTechnique(const std::string& name) noexcept;
		void Submit(uint64_t channelFilter) noexcept override;
		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}
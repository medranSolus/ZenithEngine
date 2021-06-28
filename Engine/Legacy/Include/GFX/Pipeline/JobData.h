#pragma once
#include "IRenderable.h"
#include "Technique.h"
#include "GFX/Data/BoundingBox.h"

namespace ZE::GFX::Pipeline
{
	class JobData : public virtual IRenderable
	{
	protected:
		std::vector<Technique> techniques;

		void AddTechnique(Graphics& gfx, Technique&& technique) noexcept { techniques.emplace_back(std::forward<Technique>(technique)); }

		void SetTechniques(Graphics& gfx, std::vector<Technique>&& newTechniques, const GfxObject& parent);

	public:
		JobData() = default;
		JobData(JobData&&) = default;
		JobData(const JobData&) = default;
		JobData& operator=(JobData&&) = default;
		JobData& operator=(const JobData&) = default;
		virtual ~JobData() = default;

		virtual const std::string& GetName() const noexcept = 0;
		virtual const Data::BoundingBox& GetBoundingBox() const noexcept = 0;
		virtual U32 GetIndexCount() const noexcept = 0;
		virtual void Bind(Graphics& gfx) const = 0;

		Technique* GetTechnique(const std::string& name) noexcept;
		void Submit(U64 channelFilter) const noexcept override;
		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}
#pragma once
#include "IVolume.h"

namespace ZE::GFX::Light::Volume
{
	class GlobeVolume : public IVolume
	{
	protected:
		void UpdateTransform(float volume, const Data::CBuffer::DynamicCBuffer& lightBuffer) noexcept override;

	public:
		GlobeVolume(Graphics& gfx, U32 density);
		virtual ~GlobeVolume() = default;
	};
}
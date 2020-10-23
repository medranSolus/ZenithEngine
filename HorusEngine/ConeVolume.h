#pragma once
#include "IVolume.h"

namespace GFX::Light::Volume
{
	class ConeVolume : public IVolume
	{
	protected:
		void UpdateTransform(float volume, const Data::CBuffer::DynamicCBuffer& lightBuffer) noexcept override;

	public:
		ConeVolume(Graphics& gfx, unsigned int density);
		virtual ~ConeVolume() = default;
	};
}
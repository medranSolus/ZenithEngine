#pragma once
#include "IVolume.h"

namespace GFX::Light::Volume
{
	class GlobeVolume : public IVolume
	{
	public:
		GlobeVolume(Graphics& gfx, unsigned int density);
		inline GlobeVolume(GlobeVolume&& globe) noexcept : IVolume(std::forward<IVolume&&>(globe)) {}
		inline GlobeVolume& operator=(GlobeVolume&& globe) noexcept { return static_cast<GlobeVolume&>(this->IVolume::operator=(std::forward<IVolume&&>(globe))); }
		virtual ~GlobeVolume() = default;
	};
}
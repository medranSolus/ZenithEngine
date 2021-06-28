#pragma once
#include "GFX/Probe/IProbeable.h"

namespace ZE::GFX::Pipeline
{
	class IRenderable : public Probe::IProbeable
	{
	public:
		virtual ~IRenderable() = default;

		virtual void SetOutline() noexcept = 0;
		virtual void DisableOutline() noexcept = 0;
		virtual void Submit(U64 channelFilter) const noexcept = 0;
	};
}
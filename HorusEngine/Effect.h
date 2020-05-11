#pragma once
#include "IVisual.h"
#include "GfxResources.h"

namespace GFX::Visual
{
	class Effect : public IVisual
	{
	public:
		virtual ~Effect() = default;
	};
}
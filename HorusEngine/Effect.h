#pragma once
#include "IVisual.h"

namespace GFX::Visual
{
	class Effect : public IVisual
	{
	public:
		virtual ~Effect() = default;
	};
}
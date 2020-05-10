#pragma once
#include "IProbeable.h"
#include "RenderCommander.h"

namespace GFX
{
	class IRenderable : public Probe::IProbeable
	{
	public:
		virtual ~IRenderable() = default;

		virtual void Submit(Pipeline::RenderCommander& renderer) noexcept(!IS_DEBUG) = 0;
	};
}
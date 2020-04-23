#pragma once
#include "Graphics.h"
#include "RenderCommander.h"

namespace GFX
{
	class IRenderable
	{
	public:
		virtual ~IRenderable() = default;

		virtual void Submit(Pipeline::RenderCommander& renderer) noexcept(!IS_DEBUG) = 0;
		virtual void ShowWindow(Graphics& gfx) noexcept = 0;
	};
}
#include "Viewport.h"

namespace GFX::Resource
{
	Viewport::Viewport(Graphics& gfx, unsigned int width, unsigned int height) : width(width), height(height)
	{
		viewport.Width = static_cast<FLOAT>(width);
		viewport.Height = static_cast<FLOAT>(height);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
	}
}
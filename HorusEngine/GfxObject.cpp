#include "GfxObject.h"

namespace GFX
{
	GfxObject::GfxObject(bool init)
	{
		if (init)
		{
			transform = std::make_shared<DirectX::XMFLOAT4X4>();
			scaling = std::make_shared<DirectX::XMFLOAT4X4>();
		}
	}
}
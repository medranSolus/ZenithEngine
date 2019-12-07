#include "PointLight.h"

namespace GFX::Light
{
	void PointLight::Bind(Graphics & gfx) const noexcept
	{
		buffer.Update(gfx, { mesh.GetPos() });
		buffer.Bind(gfx);
	}
}

#include "PointLight.h"
#include "Math.h"

auto mat = randColor();

namespace GFX::Light
{
	void PointLight::Bind(Graphics & gfx) const noexcept
	{
		buffer.Update(gfx,
			{
				mat,
				{ 0.1f, 0.1f, 0.1f, 0.1f },
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				mesh.GetPos(),
				1.0f,
				0.045f,
				0.0075
			});
		buffer.Bind(gfx);
	}
}

#include "IBindable.h"
#include "Graphics.h"

namespace GFX::Resource
{
	ID3D11DeviceContext* IBindable::GetContext(Graphics& gfx) noexcept
	{
		return gfx.context.Get();
	}

	ID3D11Device* IBindable::GetDevice(Graphics& gfx) noexcept
	{
		return gfx.device.Get();
	}
}
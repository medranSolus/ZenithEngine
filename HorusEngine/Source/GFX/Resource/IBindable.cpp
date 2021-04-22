#include "GFX/Resource/IBindable.h"
#include "GFX/Graphics.h"

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
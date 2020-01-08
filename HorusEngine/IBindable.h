#pragma once
#include "Graphics.h"

namespace GFX::Resource
{
	class IBindable
	{
	protected:
		inline static ID3D11DeviceContext* GetContext(Graphics & gfx) noexcept { return gfx.context.Get(); }
		inline static ID3D11Device* GetDevice(Graphics & gfx) noexcept { return gfx.device.Get(); }

	public:
		virtual ~IBindable() = default;

		virtual void Bind(Graphics& gfx) noexcept = 0;
	};
}
#pragma once
#include "IBindable.h"

namespace GFX::Resource
{
	class Topology : public IBindable
	{
	protected:
		D3D11_PRIMITIVE_TOPOLOGY type;

	public:
		Topology(Graphics & gfx, D3D11_PRIMITIVE_TOPOLOGY type) : type(type) {}

		inline void Bind(Graphics & gfx) noexcept override { GetContext(gfx)->IASetPrimitiveTopology(type); }
	};
}
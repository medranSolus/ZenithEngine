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
		Topology(const Topology&) = default;
		Topology & operator=(const Topology&) = default;
		~Topology() = default;

		inline void Bind(Graphics & gfx) noexcept override { GetContext(gfx)->IASetPrimitiveTopology(type); }
	};
}
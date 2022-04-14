#pragma once
#include "CBuffer.h"

namespace ZE::GFX::API::DX11::Resource
{
	template<typename T>
	class Constant final
	{
		CBuffer buffer;

	public:
		Constant() = default;
		Constant(GFX::Device& dev, const T& value) : buffer(dev, &value, sizeof(T), true) {}
		ZE_CLASS_MOVE(Constant);
		~Constant() = default;

		constexpr T GetData(GFX::Device& dev) const { T data; buffer.GetData(dev, &data, sizeof(T)); return data; }
		constexpr void Set(GFX::Device& dev, const T& value) const { buffer.Update(dev, &value, sizeof(T)); }

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept { buffer.Bind(cl, bindCtx); }
	};
}
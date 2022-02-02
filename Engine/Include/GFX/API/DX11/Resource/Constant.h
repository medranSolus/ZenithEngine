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
		Constant(GFX::Device& dev, const T value) : buffer(dev, &value, sizeof(value), true) {}
		ZE_CLASS_MOVE(Constant);
		~Constant() = default;

		constexpr T& GetData() noexcept { return *reinterpret_cast<T*>(buffer.GetRegion()); }
		constexpr void Set(const T& value) const { *reinterpret_cast<T*>(buffer.GetRegion()) = value; }

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept { buffer.Bind(cl, bindCtx); }
	};
}
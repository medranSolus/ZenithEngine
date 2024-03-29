#pragma once
#include "CBuffer.h"

namespace ZE::RHI::DX11::Resource
{
	template<typename T>
	class Constant final
	{
		CBuffer buffer;

	public:
		Constant() = default;
		Constant(GFX::Device& dev, const T& value) : buffer(dev.Get().dx11, &value, sizeof(T), true) {}
		ZE_CLASS_MOVE(Constant);
		~Constant() { buffer.Free(); }

		constexpr void Set(GFX::Device& dev, const T& value) const { buffer.Update(dev.Get().dx11, { INVALID_EID, &value, nullptr, sizeof(T) }); }
		constexpr T GetData(GFX::Device& dev) const { T data; buffer.GetData(dev, &data, sizeof(T)); return data; }

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept { buffer.Bind(cl, bindCtx); }
	};
}
#pragma once
#include "ConstantBuffer.h"

namespace GFX::Resource
{
	template<typename T>
	class ConstantPixelBuffer : public ConstantBuffer<T>
	{
		using ConstantBuffer<T>::ConstantBuffer;
		using ConstantBuffer<T>::GetContext;
		using ConstantBuffer<T>::constantBuffer;
		using ConstantBuffer<T>::slot;

	public:
		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->PSSetConstantBuffers(slot, 1U, constantBuffer.GetAddressOf()); }
	};
}

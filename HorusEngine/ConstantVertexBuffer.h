#pragma once
#include "ConstantBuffer.h"

namespace GFX::Resource
{
	template<typename T>
	class ConstantVertexBuffer : public ConstantBuffer<T>
	{
		using ConstantBuffer<T>::ConstantBuffer;
		using ConstantBuffer<T>::GetContext;
		using ConstantBuffer<T>::constantBuffer;

	public:
		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->VSSetConstantBuffers(0U, 1U, constantBuffer.GetAddressOf()); }
	};
}

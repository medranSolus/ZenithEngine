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
		using ConstantBuffer<T>::slot;

	public:
		ConstantVertexBuffer(const ConstantVertexBuffer&) = delete;
		ConstantVertexBuffer & operator=(const ConstantVertexBuffer&) = delete;
		~ConstantVertexBuffer() = default;

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->VSSetConstantBuffers(slot, 1U, constantBuffer.GetAddressOf()); }
	};
}

#pragma once
#include "IBindable.h"
#include "GfxExceptionMacros.h"
#include "VertexDataBuffer.h"

namespace GFX::Resource
{
	class VertexBuffer : public IBindable
	{
	protected:
		UINT stride;
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

	public:
		VertexBuffer(Graphics & gfx, const BasicType::VertexDataBuffer & buffer);
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer & operator=(const VertexBuffer&) = delete;
		~VertexBuffer() = default;

		inline void Bind(Graphics& gfx) noexcept override 
		{
			const UINT offset = 0U;
			GetContext(gfx)->IASetVertexBuffers(0U, 1U, vertexBuffer.GetAddressOf(), &stride, &offset);
		}
	};
}

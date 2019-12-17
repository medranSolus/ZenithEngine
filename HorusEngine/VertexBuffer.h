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
		template<class V>
		VertexBuffer(Graphics & gfx, const std::vector<V> & vertices) : stride(sizeof(V))
		{
			GFX_ENABLE_ALL(gfx);
			D3D11_BUFFER_DESC bufferDesc = { 0 };
			bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
			bufferDesc.CPUAccessFlags = 0U;
			bufferDesc.MiscFlags = 0U;
			bufferDesc.ByteWidth = static_cast<UINT>(sizeof(V) * vertices.size());
			bufferDesc.StructureByteStride = sizeof(V);
			D3D11_SUBRESOURCE_DATA resData = { 0 };
			resData.pSysMem = vertices.data();
			GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, &resData, &vertexBuffer));
		}

		VertexBuffer(Graphics & gfx, const BasicType::VertexDataBuffer & buffer)
			: stride(static_cast<UINT>(buffer.GetLayout().Size()))
		{
			GFX_ENABLE_ALL(gfx);
			D3D11_BUFFER_DESC bufferDesc = { 0 };
			bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
			bufferDesc.CPUAccessFlags = 0U;
			bufferDesc.MiscFlags = 0U;
			bufferDesc.ByteWidth = static_cast<UINT>(buffer.Bytes());
			bufferDesc.StructureByteStride = stride;
			D3D11_SUBRESOURCE_DATA resData = { 0 };
			resData.pSysMem = buffer.GetData();
			GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, &resData, &vertexBuffer));
		}

		inline void Bind(Graphics& gfx) noexcept override 
		{
			const UINT offset = 0U;
			GetContext(gfx)->IASetVertexBuffers(0U, 1U, vertexBuffer.GetAddressOf(), &stride, &offset);
		}
	};
}

#pragma once
#include "Codex.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	template<typename T>
	class ConstantBuffer : public IBindable
	{
	protected:
		UINT slot;
		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	public:
		ConstantBuffer(Graphics& gfx, const T & values, UINT slot = 0U);
		ConstantBuffer(Graphics & gfx, UINT slot = 0U);
		ConstantBuffer(const ConstantBuffer&) = delete;
		ConstantBuffer & operator=(const ConstantBuffer&) = delete;
		virtual ~ConstantBuffer() = default;

		constexpr UINT GetSlot() const noexcept { return slot; }

		void Update(Graphics & gfx, const T & values);
	};

	template<typename T>
	inline ConstantBuffer<T>::ConstantBuffer(Graphics & gfx, const T & values, UINT slot) : slot(slot)
	{
		GFX_ENABLE_ALL(gfx);
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0U;
		bufferDesc.ByteWidth = sizeof(values);
		bufferDesc.StructureByteStride = 0U;
		D3D11_SUBRESOURCE_DATA resData = {};
		resData.pSysMem = &values;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, &resData, &constantBuffer));
	}

	template<typename T>
	inline ConstantBuffer<T>::ConstantBuffer(Graphics & gfx, UINT slot) : slot(slot)
	{
		GFX_ENABLE_ALL(gfx);
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth = sizeof(T);
		bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0U;
		bufferDesc.StructureByteStride = 0U;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, nullptr, &constantBuffer));
	}
	
	template<typename T>
	inline void ConstantBuffer<T>::Update(Graphics & gfx, const T & values)
	{
		GFX_ENABLE_ALL(gfx);
		D3D11_MAPPED_SUBRESOURCE subres;
		GFX_THROW_FAILED(GetContext(gfx)->Map(constantBuffer.Get(), 0U, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0U, &subres));
		memcpy(subres.pData, &values, sizeof(values));
		GetContext(gfx)->Unmap(constantBuffer.Get(), 0u);
	}
}

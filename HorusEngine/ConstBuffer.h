#pragma once
#include "GfxResPtr.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	template<typename T>
	class ConstBuffer : public IBindable
	{
	protected:
		UINT slot;
		std::string name;
		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	public:
		ConstBuffer(Graphics& gfx, const std::string& tag, const T& values, UINT slot = 0U);
		ConstBuffer(Graphics& gfx, const std::string& tag, UINT slot = 0U);
		virtual ~ConstBuffer() = default;

		constexpr UINT GetSlot() const noexcept { return slot; }

		void Update(Graphics& gfx, const T& values);
	};

	template<typename T>
	inline ConstBuffer<T>::ConstBuffer(Graphics& gfx, const std::string& tag, const T& values, UINT slot)
		: slot(slot), name(tag)
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
		SET_DEBUG_NAME(constantBuffer.Get(), "CB" + std::string(typeid(T).name()) + std::to_string(slot) + "#" + tag);
	}

	template<typename T>
	inline ConstBuffer<T>::ConstBuffer(Graphics& gfx, const std::string& tag, UINT slot)
		: slot(slot), name(tag)
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
		SET_DEBUG_NAME(constantBuffer.Get(), "CB" + std::string(typeid(T).name()) + std::to_string(slot) + "#" + tag);
	}

	template<typename T>
	inline void ConstBuffer<T>::Update(Graphics& gfx, const T& values)
	{
		GFX_ENABLE_ALL(gfx);
		D3D11_MAPPED_SUBRESOURCE subres;
		GFX_THROW_FAILED(GetContext(gfx)->Map(constantBuffer.Get(), 0U, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0U, &subres));
		memcpy(subres.pData, &values, sizeof(values));
		GetContext(gfx)->Unmap(constantBuffer.Get(), 0U);
	}
}
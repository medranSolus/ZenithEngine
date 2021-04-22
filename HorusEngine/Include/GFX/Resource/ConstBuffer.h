#pragma once
#include "GfxResPtr.h"
#include "GfxDebugName.h"
#include "GFX/Graphics.h"

namespace GFX::Resource
{
	template<typename T>
	class ConstBuffer : public IBindable
	{
	protected:
		U32 slot;
		std::string name;
		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	public:
		ConstBuffer(Graphics& gfx, const std::string& tag, const T& values, U32 slot = 0);
		ConstBuffer(Graphics& gfx, const std::string& tag, U32 slot = 0);
		virtual ~ConstBuffer() = default;

		constexpr U32 GetSlot() const noexcept { return slot; }

		void Update(Graphics& gfx, const T& values);
	};

#pragma region Functions
	template<typename T>
	ConstBuffer<T>::ConstBuffer(Graphics& gfx, const std::string& tag, const T& values, U32 slot)
		: slot(slot), name(tag)
	{
		GFX_ENABLE_ALL(gfx);
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = sizeof(values);
		bufferDesc.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA resData = {};
		resData.pSysMem = &values;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, &resData, &constantBuffer));
	}

	template<typename T>
	ConstBuffer<T>::ConstBuffer(Graphics& gfx, const std::string& tag, U32 slot)
		: slot(slot), name(tag)
	{
		GFX_ENABLE_ALL(gfx);
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth = sizeof(T);
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, nullptr, &constantBuffer));
	}

	template<typename T>
	void ConstBuffer<T>::Update(Graphics& gfx, const T& values)
	{
		GFX_ENABLE_ALL(gfx);
		D3D11_MAPPED_SUBRESOURCE subres;
		GFX_THROW_FAILED(GetContext(gfx)->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subres));
		memcpy(subres.pData, &values, sizeof(values));
		GetContext(gfx)->Unmap(constantBuffer.Get(), 0);
	}
#pragma endregion
}
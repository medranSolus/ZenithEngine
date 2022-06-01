#pragma once
#include "GfxResPtr.h"
#include "GfxDebugName.h"
#include "GFX/Graphics.h"

#define ZE_BIND_CBUF(SetMethod) GetContext(gfx)->SetMethod(slot, 1, constantBuffer.GetAddressOf());

namespace ZE::GFX::Resource
{
	template<ShaderType S, typename T>
	class ConstBuffer : public IBindable
	{
		U32 slot;
		std::string name;
		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	public:
		ConstBuffer(Graphics& gfx, const std::string& tag, const T& values, U32 slot = 0);
		ConstBuffer(Graphics& gfx, const std::string& tag, U32 slot = 0);
		virtual ~ConstBuffer() = default;

		static std::string GenerateRID(const std::string& tag, const T& values, U32 slot = 0) noexcept { return GenerateRID(tag, slot); }
		static std::string GenerateRID(const std::string& tag, U32 slot = 0) noexcept;

		static GfxResPtr<ConstBuffer> Get(Graphics& gfx, const std::string& tag, const T& values, U32 slot = 0);
		static GfxResPtr<ConstBuffer> Get(Graphics& gfx, const std::string& tag, U32 slot = 0);

		template<ShaderType SD>
		constexpr operator ConstBuffer<SD, T>& () noexcept { return *static_cast<ConstBuffer<SD, T>*>(static_cast<void*>(this)); }
		template<ShaderType SD>
		constexpr operator const ConstBuffer<SD, T>& () const noexcept { return *static_cast<const ConstBuffer<SD, T>*>(static_cast<void*>(this)); }

		constexpr U32 GetSlot() const noexcept { return slot; }
		void BindVS(Graphics& gfx) const { ZE_BIND_CBUF(VSSetConstantBuffers); }
		void BindGS(Graphics& gfx) const { ZE_BIND_CBUF(GSSetConstantBuffers); }
		void BindPS(Graphics& gfx) const { ZE_BIND_CBUF(PSSetConstantBuffers); }
		void BindCompute(Graphics& gfx) const override { ZE_BIND_CBUF(CSSetConstantBuffers); }

		void Update(Graphics& gfx, const T& values);
		void Bind(Graphics& gfx) const override;
	};

	template<ShaderType S, typename T>
	struct is_resolvable_by_codex<ConstBuffer<S, T>>
	{
		static constexpr bool GENERATE_ID{ true };
	};

#pragma region Functions
	template<ShaderType S, typename T>
	ConstBuffer<S, T>::ConstBuffer(Graphics& gfx, const std::string& tag, const T& values, U32 slot)
		: slot(slot), name(tag)
	{
		ZE_GFX_ENABLE_ALL(gfx);
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = sizeof(values);
		bufferDesc.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA resData = {};
		resData.pSysMem = &values;
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, &resData, &constantBuffer));
		ZE_GFX_SET_RID(constantBuffer.Get());
	}

	template<ShaderType S, typename T>
	ConstBuffer<S, T>::ConstBuffer(Graphics& gfx, const std::string& tag, U32 slot)
		: slot(slot), name(tag)
	{
		ZE_GFX_ENABLE_ALL(gfx);
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth = sizeof(T);
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, nullptr, &constantBuffer));
		ZE_GFX_SET_RID(constantBuffer.Get());
	}

	template<ShaderType S, typename T>
	std::string ConstBuffer<S, T>::GenerateRID(const std::string& tag, U32 slot) noexcept
	{
		return "C" + std::to_string(slot) + "#" + std::to_string(sizeof(T)) + "#" + tag;
	}

	template<ShaderType S, typename T>
	GfxResPtr<ConstBuffer<S, T>> ConstBuffer<S, T>::Get(Graphics& gfx, const std::string& tag, const T& values, U32 slot)
	{
		return Codex::Resolve<ConstBuffer>(gfx, tag, values, slot);
	}

	template<ShaderType S, typename T>
	GfxResPtr<ConstBuffer<S, T>> ConstBuffer<S, T>::Get(Graphics& gfx, const std::string& tag, U32 slot)
	{
		return Codex::Resolve<ConstBuffer>(gfx, tag, slot);
	}

	template<ShaderType S, typename T>
	void ConstBuffer<S, T>::Update(Graphics& gfx, const T& values)
	{
		ZE_GFX_ENABLE_ALL(gfx);
		D3D11_MAPPED_SUBRESOURCE subres;
		ZE_GFX_THROW_FAILED(GetContext(gfx)->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subres));
		memcpy(subres.pData, &values, sizeof(values));
		GetContext(gfx)->Unmap(constantBuffer.Get(), 0);
	}

	template<ShaderType S, typename T>
	void ConstBuffer<S, T>::Bind(Graphics& gfx) const
	{
		if constexpr (S == ShaderType::Vertex)
			BindVS(gfx);
		else if constexpr (S == ShaderType::Geometry)
			BindGS(gfx);
		else if constexpr (S == ShaderType::Pixel)
			BindPS(gfx);
		else if constexpr (S == ShaderType::Compute)
			BindCompute(gfx);
		else
		{
			assert(false && "Not all ConstBuffers have defined Bind function!");
		}
	}
#pragma endregion

	template<typename T>
	using ConstBufferVertex = ConstBuffer<ShaderType::Vertex, T>;
	template<typename T>
	using ConstBufferGeometry = ConstBuffer<ShaderType::Geometry, T>;
	template<typename T>
	using ConstBufferPixel = ConstBuffer<ShaderType::Pixel, T>;
	template<typename T>
	using ConstBufferCompute = ConstBuffer<ShaderType::Compute, T>;
}

#undef ZE_BIND_CBUF
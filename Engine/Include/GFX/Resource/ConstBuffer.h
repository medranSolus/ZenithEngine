#pragma once
#include "GfxResPtr.h"
#include "GfxDebugName.h"
#include "GFX/Graphics.h"

namespace ZE::GFX::Resource
{
	template<ShaderType S, typename T>
	class ConstBuffer : public IBindable
	{
		template<ShaderType>
		struct Desc {};

#pragma region Descriptor Info
		template<> struct Desc<ShaderType::Vertex>
		{
			static constexpr const char* TYPE_PREFIX = "V";
		};
		template<> struct Desc<ShaderType::Geometry>
		{
			static constexpr const char* TYPE_PREFIX = "G";
		};
		template<> struct Desc<ShaderType::Pixel>
		{
			static constexpr const char* TYPE_PREFIX = "P";
		};
#pragma endregion

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

		constexpr U32 GetSlot() const noexcept { return slot; }

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
		return "C" + std::to_string(slot) + Desc<S>::TYPE_PREFIX + std::to_string(sizeof(T)) + "#" + tag;
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
			GetContext(gfx)->VSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());
		else if constexpr (S == ShaderType::Geometry)
			GetContext(gfx)->GSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());
		else if constexpr (S == ShaderType::Pixel)
			GetContext(gfx)->PSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());
		else
			static_assert(false, "Not all ConstBuffers have defined Bind function!");
	}
#pragma endregion

	template<typename T>
	using ConstBufferVertex = ConstBuffer<ShaderType::Vertex, T>;
	template<typename T>
	using ConstBufferGeometry = ConstBuffer<ShaderType::Geometry, T>;
	template<typename T>
	using ConstBufferPixel = ConstBuffer<ShaderType::Pixel, T>;
}
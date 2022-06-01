#pragma once
#include "GfxResPtr.h"
#include "GfxDebugName.h"
#include "GFX/Graphics.h"

#define ZE_BIND_CBUF(SetMethod) GetContext(gfx)->SetMethod(slot, 1, constantBuffer.GetAddressOf());

namespace ZE::GFX::Resource
{
	template<ShaderType T>
	class ConstBufferEx : public IBindable
	{
	protected:
		U32 slot;
		std::string name;
		const Data::CBuffer::DCBLayoutElement& rootLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	public:
		ConstBufferEx(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			U32 slot = 0, const Data::CBuffer::DynamicCBuffer* buffer = nullptr, bool debugName = true);
		virtual ~ConstBufferEx() = default;

		static std::string GenerateRID(const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			U32 slot = 0, const Data::CBuffer::DynamicCBuffer* buffer = nullptr) noexcept;

		static GfxResPtr<ConstBufferEx> Get(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			U32 slot = 0, const Data::CBuffer::DynamicCBuffer* buffer = nullptr);

		template<ShaderType S>
		constexpr operator ConstBufferEx<S>& () noexcept { return *static_cast<ConstBufferEx<S>*>(static_cast<void*>(this)); }
		template<ShaderType S>
		constexpr operator const ConstBufferEx<S>& () const noexcept { return *static_cast<const ConstBufferEx<S>*>(static_cast<void*>(this)); }

		std::string GetRID() const noexcept override { return GenerateRID(name, rootLayout, slot); }
		virtual void BindVS(Graphics& gfx) const { ZE_BIND_CBUF(VSSetConstantBuffers); }
		virtual void BindGS(Graphics& gfx) const { ZE_BIND_CBUF(GSSetConstantBuffers); }
		virtual void BindPS(Graphics& gfx) const { ZE_BIND_CBUF(PSSetConstantBuffers); }
		void BindCompute(Graphics& gfx) const override { ZE_BIND_CBUF(CSSetConstantBuffers); }

		void Update(Graphics& gfx, const Data::CBuffer::DynamicCBuffer& buffer) const;
		void Bind(Graphics& gfx) const override;
	};

	template<ShaderType T>
	struct is_resolvable_by_codex<ConstBufferEx<T>>
	{
		static constexpr bool GENERATE_ID{ true };
	};

#pragma region Functions
	template<ShaderType T>
	ConstBufferEx<T>::ConstBufferEx(Graphics& gfx, const std::string& tag,
		const Data::CBuffer::DCBLayoutElement& root, U32 slot,
		const Data::CBuffer::DynamicCBuffer* buffer, bool debugName)
		: slot(slot), name(tag), rootLayout(root)
	{
		ZE_GFX_ENABLE_ALL(gfx);
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = static_cast<UINT>(rootLayout.GetByteSize());
		bufferDesc.StructureByteStride = 0;
		if (buffer)
		{
			D3D11_SUBRESOURCE_DATA resData = {};
			resData.pSysMem = buffer->GetData();
			ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, &resData, &constantBuffer));
		}
		else
		{
			ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, nullptr, &constantBuffer));
		}
#ifdef _ZE_MODE_DEBUG
		if (debugName)
		{
			ZE_GFX_ENABLE_ALL(gfx);
			ZE_GFX_SET_RID(constantBuffer.Get());
		}
#endif
	}

	template<ShaderType T>
	std::string ConstBufferEx<T>::GenerateRID(const std::string& tag,
		const Data::CBuffer::DCBLayoutElement& root, U32 slot,
		const Data::CBuffer::DynamicCBuffer* buffer) noexcept
	{
		return "E" + std::to_string(slot) + "#" + std::to_string(root.GetByteSize()) + "#" + tag;
	}

	template<ShaderType T>
	GfxResPtr<ConstBufferEx<T>> ConstBufferEx<T>::Get(Graphics& gfx,
		const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
		U32 slot, const Data::CBuffer::DynamicCBuffer* buffer)
	{
		return Codex::Resolve<ConstBufferEx>(gfx, tag, root, slot, buffer);
	}

	template<ShaderType T>
	void ConstBufferEx<T>::Update(Graphics& gfx, const Data::CBuffer::DynamicCBuffer& buffer) const
	{
		assert(&buffer.GetRootElement() == &rootLayout);
		ZE_GFX_ENABLE_ALL(gfx);
		D3D11_MAPPED_SUBRESOURCE subres;
		ZE_GFX_THROW_FAILED(GetContext(gfx)->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subres));
		memcpy(subres.pData, buffer.GetData(), buffer.GetByteSize());
		GetContext(gfx)->Unmap(constantBuffer.Get(), 0);
	}

	template<ShaderType T>
	void ConstBufferEx<T>::Bind(Graphics& gfx) const
	{
		if constexpr (T == ShaderType::Vertex)
			BindVS(gfx);
		else if constexpr (T == ShaderType::Geometry)
			BindGS(gfx);
		else if constexpr (T == ShaderType::Pixel)
			BindPS(gfx);
		else if constexpr (T == ShaderType::Compute)
			BindCompute(gfx);
		else
		{
			assert(false && "Not all ConstBuffersEx have defined Bind function!");
		}
	}
#pragma endregion

	typedef ConstBufferEx<ShaderType::Vertex> ConstBufferExVertex;
	typedef ConstBufferEx<ShaderType::Geometry> ConstBufferExGeometry;
	typedef ConstBufferEx<ShaderType::Pixel> ConstBufferExPixel;
	typedef ConstBufferEx<ShaderType::Compute> ConstBufferExCompute;
}

#undef ZE_BIND_CBUF
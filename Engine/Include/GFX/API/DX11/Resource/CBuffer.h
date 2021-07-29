#pragma once
#include "GFX/API/DX/GraphicsException.h"
#include "GFX/Context.h"
#include "GFX/ShaderSlot.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource
{
	class CBuffer final
	{
		DX::ComPtr<ID3D11Buffer> buffer;

	public:
		CBuffer(GFX::Device& dev, U8* values, U32 bytes, bool dynamic);
		CBuffer(CBuffer&&) = default;
		CBuffer(const CBuffer&) = delete;
		CBuffer& operator=(CBuffer&&) = default;
		CBuffer& operator=(const CBuffer&) = delete;
		~CBuffer() = default;

		template<bool IsDynamic>
		void Update(GFX::Device& dev, GFX::Context& ctx, U8* values, U64 bytes) const;

		void BindVS(GFX::Context& ctx, ShaderSlot slot) const noexcept;
		void BindDS(GFX::Context& ctx, ShaderSlot slot) const noexcept;
		void BindHS(GFX::Context& ctx, ShaderSlot slot) const noexcept;
		void BindGS(GFX::Context& ctx, ShaderSlot slot) const noexcept;
		void BindPS(GFX::Context& ctx, ShaderSlot slot) const noexcept;
		void BindCS(GFX::Context& ctx, ShaderSlot slot) const noexcept;
	};

#pragma region Functions
	template<bool IsDynamic>
	void CBuffer::Update(GFX::Device& dev, GFX::Context& ctx, U8* values, U64 bytes) const
	{
		if constexpr (IsDynamic)
		{
			ZE_GFX_ENABLE(dev.Get().dx11);
			D3D11_MAPPED_SUBRESOURCE subres;
			ZE_GFX_THROW_FAILED(ctx.Get().dx11.GetContext()->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subres));
			memcpy(subres.pData, values, bytes);
			ctx.Get().dx11.GetContext()->Unmap(buffer.Get(), 0);
		}
		else
			ctx.Get().dx11.GetContext()->UpdateSubresource(buffer.Get(), 0, nullptr, values, 0, 0);
	}
#pragma endregion
}
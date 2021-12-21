#pragma once
#include "GFX/API/DX/GraphicsException.h"
#include "GFX/CommandList.h"
#include "GFX/ShaderSlot.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource
{
	class CBuffer final
	{
		DX::ComPtr<ID3D11Buffer> buffer;

	public:
		CBuffer(GFX::Device& dev, U8* values, U32 bytes, bool dynamic);
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() = default;

		void Update(GFX::CommandList& cl, U8* values, U64 bytes) const;
		void UpdateDynamic(GFX::Device& dev, GFX::CommandList& cl, U8* values, U64 bytes) const;

		void BindVS(GFX::CommandList& cl, ShaderSlot slot) const noexcept;
		void BindDS(GFX::CommandList& cl, ShaderSlot slot) const noexcept;
		void BindHS(GFX::CommandList& cl, ShaderSlot slot) const noexcept;
		void BindGS(GFX::CommandList& cl, ShaderSlot slot) const noexcept;
		void BindPS(GFX::CommandList& cl, ShaderSlot slot) const noexcept;
		void BindCS(GFX::CommandList& cl, ShaderSlot slot) const noexcept;
	};
}
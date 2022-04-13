#pragma once
#include "GFX/Device.h"
#include "GFX/Binding/Context.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource
{
	class CBuffer final
	{
		DX::ComPtr<ID3D11Buffer> buffer;
		bool dynamic;

	public:
		CBuffer() = default;
		CBuffer(GFX::Device& dev, const void* values, U32 bytes, bool dynamic);
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() = default;

		void* GetRegion(GFX::Device& dev) const;
		void FlushRegion(GFX::Device& dev) const noexcept;

		void Update(GFX::Device& dev, const void* values, U32 bytes) const;
		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		void Free(GFX::Device& dev) noexcept;
	};
}
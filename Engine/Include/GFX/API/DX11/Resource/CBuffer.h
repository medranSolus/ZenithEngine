#pragma once
#include "GFX/Device.h"
#include "GFX/Binding/Context.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource
{
	class CBuffer final
	{
		DX::ComPtr<ID3D11Buffer> buffer;
		void* bufferData = nullptr;

	public:
		CBuffer() = default;
		CBuffer(GFX::Device& dev, const void* values, U32 bytes, bool dynamic);
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() = default;

		constexpr void* GetRegion() const noexcept { ZE_ASSERT(bufferData, "CBuffer is not dynamic!"); return bufferData; }

		void Update(GFX::Device& dev, const void* values, U32 bytes) const;
		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		void Free(GFX::Device& dev) noexcept;
	};
}
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
		CBuffer(GFX::Device& dev, const U8* values, U32 bytes, bool dynamic);
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() = default;

		void Update(GFX::Device& dev, GFX::CommandList& cl, const U8* values, U32 bytes) const;
		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
	};
}
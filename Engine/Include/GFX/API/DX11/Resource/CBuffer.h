#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::DX11::Resource
{
	class CBuffer final
	{
		DX::ComPtr<IBuffer> buffer;
		bool dynamic;

	public:
		CBuffer() = default;
		CBuffer(GFX::Device& dev, const void* values, U32 bytes, bool dynamic = false);
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() { ZE_ASSERT(buffer == nullptr, "Resource not freed before deletion!"); }

		void Free(GFX::Device& dev) noexcept { buffer = nullptr; }

		void Update(GFX::Device& dev, const void* values, U32 bytes) const;
		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		void GetData(GFX::Device& dev, void* values, U32 bytes) const;
	};
}
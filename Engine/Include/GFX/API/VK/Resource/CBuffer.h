#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::VK::Resource
{
	class CBuffer final
	{
	public:
		CBuffer() = default;
		CBuffer(GFX::Device& dev, const void* values, U32 bytes, bool dynamic = false);
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() = default;

		void Update(GFX::Device& dev, const void* values, U32 bytes) const;
		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		void Free(GFX::Device& dev) noexcept;
		void GetData(GFX::Device& dev, void* values, U32 bytes) const;
	};
}
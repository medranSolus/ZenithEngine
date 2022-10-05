#pragma once
#include "GFX/Resource/DynamicBufferAlloc.h"
#include "GFX/Binding/Context.h"
#include "Data/Library.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::VK::Resource
{
	class DynamicCBuffer final
	{
		void AllocBlock(GFX::Device& dev);

	public:
		DynamicCBuffer() = default;
		DynamicCBuffer(GFX::Device& dev) { AllocBlock(dev); }
		ZE_CLASS_MOVE(DynamicCBuffer);
		~DynamicCBuffer() = default;

		void Free(GFX::Device& dev) noexcept {}

		GFX::Resource::DynamicBufferAlloc Alloc(GFX::Device& dev, const void* values, U32 bytes);
		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, const GFX::Resource::DynamicBufferAlloc& allocInfo) const noexcept;
		void StartFrame(GFX::Device& dev);
	};
}
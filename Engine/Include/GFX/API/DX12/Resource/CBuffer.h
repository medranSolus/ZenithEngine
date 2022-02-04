#pragma once
#include "GFX/Device.h"
#include "GFX/Binding/Context.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Resource
{
	class CBuffer final
	{
		D3D12_GPU_VIRTUAL_ADDRESS address;
		ResourceInfo resInfo;
		void* buffer = nullptr;

	public:
		CBuffer() = default;
		CBuffer(GFX::Device& dev, const void* values, U32 bytes, bool dynamic);
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() = default;

		constexpr void* GetRegion() const noexcept { ZE_ASSERT(buffer, "CBuffer is not dynamic!"); return buffer; }

		void Update(GFX::Device& dev, const void* values, U32 bytes) const;
		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		void Free(GFX::Device& dev) noexcept;
	};
}
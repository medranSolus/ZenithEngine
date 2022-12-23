#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::DX12::Resource
{
	class CBuffer final
	{
		D3D12_GPU_VIRTUAL_ADDRESS address;
		ResourceInfo resInfo;

	public:
		CBuffer() = default;
		CBuffer(GFX::Device& dev, const void* values, U32 bytes);
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() { ZE_ASSERT(resInfo.IsFree(), "Resource not freed before deletion!"); }

		void Free(GFX::Device& dev) noexcept { dev.Get().dx12.FreeBuffer(resInfo); }

		void Update(GFX::Device& dev, const void* values, U32 bytes) const;
		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		void GetData(GFX::Device& dev, void* values, U32 bytes) const;
	};
}
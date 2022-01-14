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
		CBuffer(GFX::Device& dev, const U8* values, U32 bytes, bool dynamic);
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() = default;

		void Update(GFX::Device& dev, GFX::CommandList& cl, const U8* values, U32 bytes) const;
		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
	};
}
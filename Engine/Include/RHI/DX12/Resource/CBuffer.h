#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Resource/CBufferData.h"
#include "GFX/CommandList.h"
#include "GFX/GFile.h"

namespace ZE::RHI::DX12::Resource
{
	class CBuffer final
	{
		D3D12_GPU_VIRTUAL_ADDRESS address;
		ResourceInfo resInfo;

	public:
		CBuffer() = default;
		CBuffer(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::CBufferData& data);
		CBuffer(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::CBufferFileData& data, GFX::GFile& file);
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() { ZE_ASSERT_FREED(resInfo.IsFree()); }

		void Free(GFX::Device& dev) noexcept { dev.Get().dx12.FreeBuffer(resInfo); }

		void Update(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::CBufferData& data) const;
		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		void GetData(GFX::Device& dev, void* values, U32 bytes) const;

		// Gfx API Internal

		IResource* GetResource() const noexcept { return resInfo.Resource.Get(); }
	};
}
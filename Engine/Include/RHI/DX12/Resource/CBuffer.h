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
		Device* srcDev = nullptr;

	public:
		CBuffer() = default;
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer();

		static Expected<CBuffer> Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::CBufferData& data) noexcept;
		static Expected<CBuffer> Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::CBufferFileData& data, GFX::GFile& file) noexcept;

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		Status Update(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::CBufferData& data) const noexcept;

		// Gfx API Internal

		IResource* GetResource() const noexcept { return resInfo.Resource.Get(); }
	};
}
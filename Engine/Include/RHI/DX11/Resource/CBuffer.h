#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Resource/CBufferData.h"
#include "GFX/CommandList.h"
#include "IO/File.h"

namespace ZE::RHI::DX11::Resource
{
	class CBuffer final
	{
		DX::ComPtr<IBuffer> buffer;
		bool dynamic;

	public:
		CBuffer() = default;
		CBuffer(Device& dev, const void* values, U32 bytes, bool dynamic);
		CBuffer(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::CBufferData& data);
		CBuffer(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::CBufferFileData& data, IO::File& file);
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() { ZE_ASSERT_FREED(buffer == nullptr); }

		void Free(GFX::Device& dev) noexcept { Free(); }
		void Update(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::CBufferData& data) const { Update(dev.Get().dx11, data); }

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		void GetData(GFX::Device& dev, void* values, U32 bytes) const;

		// Gfx API Internal

		void Free() noexcept { buffer = nullptr; }

		void Update(Device& dev, const GFX::Resource::CBufferData& data) const;
	};
}
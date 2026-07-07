#pragma once
#include "Data/ResourceLocation.h"
#include "GFX/Binding/Context.h"
#include "GFX/Resource/CBufferData.h"
#include "GFX/CommandList.h"
#include "GFX/GFile.h"

namespace ZE::RHI::DX11::Resource
{
	Expected<DX::ComPtr<IBuffer>> CreateCBuffer(D3D11_USAGE usage, U32 cpuAccess, Device& dev, const void* data, U32 bytes) noexcept;
	void BindCBuffer(ID3D11Buffer* const* buffer, GFX::CommandList& cl, GFX::Binding::Context& bindCtx) noexcept;

	template<bool DYNAMIC>
	class CBufferInternal final
	{
		DX::ComPtr<IBuffer> buffer;

	public:
		CBufferInternal() = default;
		ZE_CLASS_MOVE(CBufferInternal);
		~CBufferInternal() = default;

		static Expected<CBufferInternal> Create(Device& dev, const void* data, U32 bytes) noexcept;

		IBuffer* GetBuffer() const noexcept { return buffer.Get(); }
		DX::ComPtr<IResource> GetResource() const noexcept { return buffer; }
		void SetBuffer(DX::ComPtr<IBuffer> newBuffer) noexcept { buffer = std::move(newBuffer); }
		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept { BindCBuffer(buffer.GetAddressOf(), cl, bindCtx); }
		
		Status Update(Device& dev, const GFX::Resource::CBufferData& data) const noexcept;
	};

	class CBuffer final
	{
		CBufferInternal<false> impl;

	public:
		CBuffer() = default;
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() = default;

		static Expected<CBuffer> Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::CBufferData& data) noexcept;
		static Expected<CBuffer> Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::CBufferFileData& data, GFX::GFile& file) noexcept;

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept { impl.Bind(cl, bindCtx); }
		Status Update(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::CBufferData& data) const noexcept { return Update(dev.Get().dx11, data); }

		// Gfx API Internal

		IBuffer* GetBuffer() const noexcept { return impl.GetBuffer(); }
		Status Update(Device& dev, const GFX::Resource::CBufferData& data) const noexcept { return impl.Update(dev, data); }
	};

#pragma region Functions
	template<bool DYNAMIC>
	Expected<CBufferInternal<DYNAMIC>> CBufferInternal<DYNAMIC>::Create(Device& dev, const void* data, U32 bytes) noexcept
	{
		D3D11_USAGE usage;
		U32 cpuAccess;
		if constexpr (DYNAMIC)
		{
			usage = D3D11_USAGE_DYNAMIC;
			cpuAccess = D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			usage = D3D11_USAGE_DEFAULT;
			cpuAccess = 0;
		}
		
		CBufferInternal cbuff = {};
		ZE_EXPECT_RET_FAILED(cbuff.buffer, CreateCBuffer(usage, cpuAccess, dev, data, bytes));
		return cbuff;
	}

	template<bool DYNAMIC>
	Status CBufferInternal<DYNAMIC>::Update(Device& dev, const GFX::Resource::CBufferData& data) const noexcept
	{
		const void* dataSrc = data.DataRef.get() ? data.DataRef.get() : data.DataStatic;
		if constexpr (DYNAMIC)
		{
			D3D11_MAPPED_SUBRESOURCE subres;
			ZE_DX_RET_FAILED(dev.GetMainContext()->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subres));
			std::memcpy(subres.pData, dataSrc, data.Bytes);
			dev.GetMainContext()->Unmap(buffer.Get(), 0);
		}
		else
		{
			dev.GetMainContext()->UpdateSubresource(buffer.Get(), 0, nullptr, dataSrc, 0, 0);

			if (data.ResourceID != INVALID_EID)
				Settings::Data.get<Data::ResourceLocationAtom>(data.ResourceID) = Data::ResourceLocation::GPU;
		}
		return {};
	}
#pragma endregion
}
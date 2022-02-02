#pragma once
#include "GFX/Device.h"
#include "GFX/IndexData.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Resource
{
	class IndexBuffer final
	{
		D3D12_INDEX_BUFFER_VIEW view;
		ResourceInfo info;

	public:
		IndexBuffer() = default;
		IndexBuffer(GFX::Device& dev, const IndexData& data);
		ZE_CLASS_MOVE(IndexBuffer);
		~IndexBuffer() = default;

		constexpr U32 GetCount() const noexcept { return view.SizeInBytes / sizeof(U32); }
		void Bind(GFX::CommandList& cl) const noexcept { cl.Get().dx12.GetList()->IASetIndexBuffer(&view); }
		void Free(GFX::Device& dev) noexcept { view.SizeInBytes = 0; dev.Get().dx12.FreeBuffer(info, view.SizeInBytes); }

		IndexData GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}
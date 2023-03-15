#pragma once
#include "GFX/CommandList.h"
#include "GFX/IndexData.h"

namespace ZE::GFX::API::DX12::Resource
{
	class IndexBuffer final
	{
		D3D12_INDEX_BUFFER_VIEW view;
		ResourceInfo info;
		bool is16bit;

	public:
		IndexBuffer() = default;
		IndexBuffer(GFX::Device& dev, const IndexData& data);
		ZE_CLASS_MOVE(IndexBuffer);
		~IndexBuffer() { ZE_ASSERT(info.IsFree(), "Resource not freed before deletion!"); }

		constexpr U32 GetCount() const noexcept { return view.SizeInBytes / (is16bit ? sizeof(U16) : sizeof(U32)); }
		void Bind(GFX::CommandList& cl) const noexcept { cl.Get().dx12.GetList()->IASetIndexBuffer(&view); }
		void Free(GFX::Device& dev) noexcept { dev.Get().dx12.FreeBuffer(info); }

		IndexData GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}
#pragma once
#include "GFX/CommandList.h"
#include "GFX/VertexData.h"

namespace ZE::RHI::DX11::Resource
{
	class VertexBuffer final
	{
		DX::ComPtr<IBuffer> buffer;
		UINT byteStride;

	public:
		VertexBuffer() = default;
		VertexBuffer(GFX::Device& dev, const GFX::VertexData& data);
		ZE_CLASS_MOVE(VertexBuffer);
		~VertexBuffer() { ZE_ASSERT(byteStride == 0 && buffer == nullptr, "Resource not freed before deletion!"); }

		void Free(GFX::Device& dev) noexcept { byteStride = 0; buffer = nullptr; }

		void Bind(GFX::CommandList& cl) const noexcept;
		GFX::VertexData GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}
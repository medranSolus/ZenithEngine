#pragma once
#include "GFX/IndexData.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::VK::Resource
{
	class IndexBuffer final
	{
	public:
		IndexBuffer() = default;
		IndexBuffer(GFX::Device& dev, const IndexData& data);
		ZE_CLASS_MOVE(IndexBuffer);
		~IndexBuffer() = default;

		constexpr U32 GetCount() const noexcept { return 0; }
		void Free(GFX::Device& dev) noexcept {}

		void Bind(GFX::CommandList& cl) const noexcept;
		IndexData GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}
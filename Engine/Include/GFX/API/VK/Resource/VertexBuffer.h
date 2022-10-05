#pragma once
#include "GFX/VertexData.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::VK::Resource
{
	class VertexBuffer final
	{
	public:
		VertexBuffer() = default;
		VertexBuffer(GFX::Device& dev, const VertexData& data);
		ZE_CLASS_MOVE(VertexBuffer);
		~VertexBuffer() = default;

		void Free(GFX::Device& dev) noexcept {}

		void Bind(GFX::CommandList& cl) const noexcept;
		VertexData GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}
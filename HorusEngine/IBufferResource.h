#pragma once
#include "IBindable.h"

namespace GFX::Pipeline::Resource
{
	class IBufferResource : public GFX::Resource::IBindable
	{
	public:
		IBufferResource() = default;
		IBufferResource(const IBufferResource&) = delete;
		IBufferResource& operator=(const IBufferResource&) = delete;
		virtual ~IBufferResource() = default;

		virtual void Clear(Graphics& gfx) noexcept = 0;
		virtual inline void Clear(Graphics& gfx, const Data::ColorFloat4& color) noexcept { Clear(gfx); }
	};
}
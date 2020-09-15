#pragma once
#include "DepthStencil.h"

namespace GFX::Pipeline::Resource
{
	class IRenderTarget : public IBufferResource
	{
		using IBufferResource::IBufferResource;

	public:
		virtual ~IRenderTarget() = default;

		inline void Clear(Graphics& gfx) noexcept override { Clear(gfx, { 0.0f, 0.0f, 0.0f, 0.0f }); }
		void Clear(Graphics& gfx, const Data::ColorFloat4& color) noexcept override = 0;

		virtual void BindTarget(Graphics& gfx) = 0;
		virtual void BindTarget(Graphics& gfx, DepthStencil& depthStencil) = 0;
		inline void Bind(Graphics& gfx) override { BindTarget(gfx); }

		virtual Surface ToSurface(Graphics& gfx) const = 0;
	};
}
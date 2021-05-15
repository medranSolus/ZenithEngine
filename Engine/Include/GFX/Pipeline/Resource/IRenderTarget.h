#pragma once
#include "DepthStencil.h"

namespace ZE::GFX::Pipeline::Resource
{
	class IRenderTarget : public IBufferResource
	{
		using IBufferResource::IBufferResource;

	public:
		IRenderTarget(IRenderTarget&&) = default;
		IRenderTarget& operator=(IRenderTarget&&) = default;
		virtual ~IRenderTarget() = default;

		void Clear(Graphics& gfx) override { Clear(gfx, { 0.0f, 0.0f, 0.0f, 0.0f }); }
		void Clear(Graphics& gfx, const ColorF4& color) override = 0;

		virtual void BindTarget(Graphics& gfx) const = 0;
		virtual void BindTarget(Graphics& gfx, DepthStencil& depthStencil) const = 0;
		void Bind(Graphics& gfx) const override { BindTarget(gfx); }
	};
}
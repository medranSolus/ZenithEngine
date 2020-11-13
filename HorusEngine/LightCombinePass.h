#pragma once
#include "FullscreenPass.h"
#include "ConstBufferExCache.h"

namespace GFX::Pipeline::RenderPass
{
	class LightCombinePass : public Base::FullscreenPass
	{
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> ambientBuffer;

		static inline Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		LightCombinePass(Graphics& gfx, const std::string& name);
		virtual ~LightCombinePass() = default;

		void ShowWindow(Graphics& gfx);
	};
}
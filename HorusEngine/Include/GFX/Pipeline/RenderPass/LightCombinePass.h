#pragma once
#include "GFX/Pipeline/RenderPass/Base/FullscreenPass.h"
#include "GFX/Resource/ConstBufferExCache.h"

namespace GFX::Pipeline::RenderPass
{
	class LightCombinePass : public Base::FullscreenPass
	{
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> ambientBuffer;

		static Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		LightCombinePass(Graphics& gfx, std::string&& name);
		virtual ~LightCombinePass() = default;

		void ShowWindow(Graphics& gfx);
	};
}
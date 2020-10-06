#pragma once
#include "FullscreenPass.h"
#include "ConstBufferExCache.h"

namespace GFX::Pipeline::RenderPass
{
	class GammaCorrectionPass : public Base::FullscreenPass
	{
	public:
		GammaCorrectionPass(Graphics& gfx, const std::string& name);
		virtual ~GammaCorrectionPass() = default;
	};
}
#pragma once
#include "FullscreenPass.h"

namespace GFX::Pipeline::RenderPass
{
	class HDRGammaCorrectionPass : public Base::FullscreenPass
	{
	public:
		HDRGammaCorrectionPass(Graphics& gfx, const std::string& name);
		virtual ~HDRGammaCorrectionPass() = default;
	};
}
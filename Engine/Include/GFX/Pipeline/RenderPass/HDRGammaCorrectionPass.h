#pragma once
#include "GFX/Pipeline/RenderPass/Base/FullscreenPass.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	class HDRGammaCorrectionPass : public Base::FullscreenPass
	{
	public:
		HDRGammaCorrectionPass(Graphics& gfx, std::string&& name);
		virtual ~HDRGammaCorrectionPass() = default;
	};
}
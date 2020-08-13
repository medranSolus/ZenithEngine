#pragma once
#include "QueuePass.h"

namespace GFX::Pipeline::RenderPass
{
	class LambertianPass : public Base::QueuePass
	{
	public:
		LambertianPass(Graphics& gfx, const std::string& name);
		virtual ~LambertianPass() = default;
	};
}
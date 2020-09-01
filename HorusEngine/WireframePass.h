#pragma once
#include "QueuePass.h"

namespace GFX::Pipeline::RenderPass
{
	class WireframePass : public Base::QueuePass
	{
	public:
		WireframePass(Graphics& gfx, const std::string& name);
		virtual ~WireframePass() = default;
	};
}

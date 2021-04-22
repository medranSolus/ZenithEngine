#pragma once
#include "GFX/Pipeline/RenderPass/Base/QueuePass.h"

namespace GFX::Pipeline::RenderPass
{
	class WireframePass : public Base::QueuePass
	{
	public:
		WireframePass(Graphics& gfx, std::string&& name);
		virtual ~WireframePass() = default;
	};
}

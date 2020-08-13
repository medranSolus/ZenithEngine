#pragma once
#include "QueuePass.h"

namespace GFX::Pipeline::RenderPass
{
	class OutlineGenerationPass : public Base::QueuePass
	{
	public:
		OutlineGenerationPass(Graphics& gfx, const std::string& name);
		virtual ~OutlineGenerationPass() = default;
	};
}
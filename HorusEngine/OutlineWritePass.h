#pragma once
#include "QueuePass.h"

namespace GFX::Pipeline::RenderPass
{
	class OutlineWritePass : public Base::QueuePass
	{
	public:
		OutlineWritePass(Graphics& gfx, const std::string& name);
		virtual ~OutlineWritePass() = default;
	};
}
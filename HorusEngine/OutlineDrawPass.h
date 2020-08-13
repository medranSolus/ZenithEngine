#pragma once
#include "QueuePass.h"

namespace GFX::Pipeline::RenderPass
{
	class OutlineDrawPass : public Base::QueuePass
	{
	public:
		OutlineDrawPass(Graphics& gfx, const std::string& name);
		virtual ~OutlineDrawPass() = default;
	};
}
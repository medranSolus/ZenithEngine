#pragma once
#include "QueuePass.h"

namespace GFX::Pipeline::RenderPass
{
	class OutlineDrawBlurPass : public Base::QueuePass
	{
	public:
		OutlineDrawBlurPass(Graphics& gfx, const std::string& name, unsigned int width, unsigned int height);
		virtual ~OutlineDrawBlurPass() = default;

		void Execute(Graphics& gfx) override;
	};
}
#pragma once
#include "GFX/Pipeline/RenderPass/Base/QueuePass.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	class OutlineDrawBlurPass : public Base::QueuePass
	{
	public:
		OutlineDrawBlurPass(Graphics& gfx, std::string&& name, U32 width, U32 height);
		virtual ~OutlineDrawBlurPass() = default;

		void Execute(Graphics& gfx) override;
	};
}
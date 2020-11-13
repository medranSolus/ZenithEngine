#pragma once
#include "BasePass.h"

namespace GFX::Pipeline::RenderPass
{
	class ClearBufferPass : public Base::BasePass
	{
		GfxResPtr<Resource::IBufferResource> buffer;

	public:
		ClearBufferPass(const std::string& name);
		virtual ~ClearBufferPass() = default;

		inline void Execute(Graphics& gfx) override { buffer->Clear(gfx); }
	};
}
#pragma once
#include "Base/BasePass.h"

namespace GFX::Pipeline::RenderPass
{
	class ClearBufferPass : public Base::BasePass
	{
		GfxResPtr<Resource::IBufferResource> buffer;

	public:
		ClearBufferPass(std::string&& name);
		ClearBufferPass(ClearBufferPass&&) = default;
		ClearBufferPass(const ClearBufferPass&) = default;
		ClearBufferPass& operator=(ClearBufferPass&&) = default;
		ClearBufferPass& operator=(const ClearBufferPass&) = default;
		virtual ~ClearBufferPass() = default;

		void Execute(Graphics& gfx) override { buffer->Clear(gfx); }
	};
}
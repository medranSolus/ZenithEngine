#pragma once
#include "BasePass.h"

namespace GFX::Pipeline::RenderPass
{
	class ClearBufferPass : public Base::BasePass
	{
		std::shared_ptr<Resource::IBufferResource> buffer;

	public:
		ClearBufferPass(const std::string& name);
		virtual ~ClearBufferPass() = default;

		inline void Execute(Graphics& gfx) noexcept(!IS_DEBUG) override { buffer->Clear(gfx); }
	};
}
#include "LightingPass.h"
#include "RenderPassesBase.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	LightingPass::LightingPass(Graphics& gfx, const std::string& name) : QueuePass(name)
	{
		shadowMapPass = std::make_unique<ShadowMapPass>(gfx, "shadowMap");
		RegisterSink(Base::SinkDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));
		RegisterSource(Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));
	}

	void LightingPass::Reset() noexcept
	{
		shadowMapPass->Reset();
		QueuePass::Reset();
	}

	Base::BasePass& LightingPass::GetInnerPass(std::deque<std::string> nameChain)
	{
		if (nameChain.size() == 1 && nameChain.front() == shadowMapPass->GetName())
			return *shadowMapPass;
		throw RGC_EXCEPT("Wrong inner pass name: " + nameChain.front());
	}

	void LightingPass::Execute(Graphics& gfx)
	{
		shadowMapPass->Execute(gfx);
	}
}
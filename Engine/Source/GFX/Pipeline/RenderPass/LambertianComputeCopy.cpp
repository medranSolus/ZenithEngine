#include "GFX/Pipeline/RenderPass/LambertianComputeCopy.h"

namespace ZE::GFX::Pipeline::RenderPass::LambertianComputeCopy
{
	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ Base(CorePassType::LambertianComputeCopy) };
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		return desc;
	}

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Lambertian Compute Copy");
		Resources ids = *passData.Resources.CastConst<Resources>();

		ZE_DRAW_TAG_BEGIN(dev, cl, "Copy gbuffer for async SSAO", PixelVal::White);
		renderData.Buffers.CopyFullResource(cl, ids.SourceDepth, ids.CopyDepth);
		renderData.Buffers.CopyFullResource(cl, ids.SourceNormal, ids.CopyNormal);
		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}
}
#include "GFX/Pipeline/RenderPass/LambertianComputeCopy.h"

namespace ZE::GFX::Pipeline::RenderPass::LambertianComputeCopy
{
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Lambertian Compute Copy");
		const Resources ids = *passData.Buffers.CastConst<Resources>();
		cl.Open(dev);
		ZE_DRAW_TAG_BEGIN(dev, cl, "Copy gbuffer for SSAO", PixelVal::White);
		renderData.Buffers.Copy(cl, ids.SourceNormal, ids.CopyNormal);
		renderData.Buffers.Copy(cl, ids.SourceDepth, ids.CopyDepth);
		ZE_DRAW_TAG_END(dev, cl);

		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}
#include "GFX/Pipeline/RenderPass/CopyDepthTemporal.h"

namespace ZE::GFX::Pipeline::RenderPass::CopyDepthTemporal
{
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Copy Depth Temporal");
		const Resources ids = *passData.Buffers.CastConst<Resources>();
		cl.Open(dev);
		ZE_DRAW_TAG_BEGIN(dev, cl, "Copy current depth for next frame", PixelVal::White);
		renderData.Buffers.Copy(cl, ids.SourceDepth, ids.CopyDepth);
		ZE_DRAW_TAG_END(dev, cl);

		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}
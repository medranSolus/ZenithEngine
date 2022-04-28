#include "GFX/Pipeline/RenderPass/LambertianDepthCopy.h"

namespace ZE::GFX::Pipeline::RenderPass::LambertianDepthCopy
{
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		const Resources ids = *passData.Buffers.CastConst<Resources>();
		cl.Open(dev);
		ZE_DRAW_TAG_BEGIN(cl, L"Copy depth buffer for SSAO", PixelVal::White);
		renderData.Buffers.Copy(cl, ids.SourceDepth, ids.DepthCopyCompute);
		ZE_DRAW_TAG_END(cl);
		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}
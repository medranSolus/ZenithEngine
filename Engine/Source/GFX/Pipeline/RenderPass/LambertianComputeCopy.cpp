#include "GFX/Pipeline/RenderPass/LambertianComputeCopy.h"

namespace ZE::GFX::Pipeline::RenderPass::LambertianComputeCopy
{
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		const Resources ids = *passData.Buffers.CastConst<Resources>();

		ZE_DRAW_TAG_BEGIN(cl, L"Copy gbuffer for SSAO", PixelVal::White);
		renderData.Buffers.Copy(cl, ids.SourceDepth, ids.DepthCopy);
		renderData.Buffers.Copy(cl, ids.SourceNormal, ids.NormalCopy);
		ZE_DRAW_TAG_END(cl);
	}
}
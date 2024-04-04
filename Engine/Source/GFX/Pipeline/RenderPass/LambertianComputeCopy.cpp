#include "GFX/Pipeline/RenderPass/LambertianComputeCopy.h"

namespace ZE::GFX::Pipeline::RenderPass::LambertianComputeCopy
{
	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ static_cast<PassType>(CorePassType::LambertianComputeCopy) };
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		return desc;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Lambertian Compute Copy");
		Resources ids = *passData.Resources.CastConst<Resources>();

		ZE_DRAW_TAG_BEGIN(dev, cl, "Copy gbuffer for SSAO", PixelVal::White);
		//renderData.Buffers.Copy(cl, ids.SourceDepth, ids.CopyDepth);
		//renderData.Buffers.Copy(cl, ids.SourceNormal, ids.CopyNormal);
		ZE_DRAW_TAG_END(dev, cl);
	}
}
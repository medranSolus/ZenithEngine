#include "GFX/Pipeline/RenderPass/Base/ComputePass.h"
#include "GFX/Resource/Shader.h"

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	ComputePass::ComputePass(Graphics& gfx, std::string&& name, const std::string& shaderName)
		: BindingPass(std::forward<std::string>(name))
	{
		AddBind(GFX::Resource::ComputeShader::Get(gfx, shaderName));
	}

	void ComputePass::Compute(Graphics& gfx, U32 groupX, U32 groupY, U32 groupZ)
	{
		ZE_DRAW_TAG_START(gfx, GetName());
		computeTarget->Unbind(gfx);
		computeTarget->BindComputeTarget(gfx);
		BindComputeAll(gfx);
		gfx.Compute(groupX, groupY, groupZ);
		ZE_DRAW_TAG_END(gfx);
	}

	void ComputePass::ComputeFrame(Graphics& gfx, U32 threadsX, U32 threadsY)
	{
		ZE_DRAW_TAG_START(gfx, GetName());
		computeTarget->Unbind(gfx);
		computeTarget->BindComputeTarget(gfx);
		BindComputeAll(gfx);
		gfx.ComputeFrame(threadsX, threadsY);
		ZE_DRAW_TAG_END(gfx);
	}

	void ComputePass::Finalize()
	{
		if (computeTarget == nullptr)
			throw ZE_RGC_EXCEPT("ComputePass \"" + GetName() + "\" needs output target!");
		BindingPass::Finalize();
	}
}
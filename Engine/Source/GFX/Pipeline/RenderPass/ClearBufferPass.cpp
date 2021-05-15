#include "GFX/Pipeline/RenderPass/ClearBufferPass.h"
#include "GFX/Pipeline/RenderPass/Base/RenderPassesBase.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	ClearBufferPass::ClearBufferPass(std::string&& name)
		: BasePass(std::forward<std::string>(name))
	{
		RegisterSink(Base::SinkDirectBuffer<Resource::IBufferResource>::Make("buffer", buffer));
		RegisterSource(Base::SourceDirectBuffer<Resource::IBufferResource>::Make("buffer", buffer));
	}
}
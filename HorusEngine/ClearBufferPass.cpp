#include "ClearBufferPass.h"
#include "RenderPassesBase.h"

namespace GFX::Pipeline::RenderPass
{
	ClearBufferPass::ClearBufferPass(const std::string& name) : BasePass(name)
	{
		RegisterSink(Base::SinkDirectBuffer<Resource::IBufferResource>::Make("buffer", buffer));
		RegisterSource(Base::SourceDirectBuffer<Resource::IBufferResource>::Make("buffer", buffer));
	}
}
#include "GFX/Pipeline/FrameBufferDesc.h"
#include "Exception/RenderGraphCompileException.h"
#include "Settings.h"

namespace ZE::GFX::Pipeline
{
	void FrameBufferDesc::Init(U64 resourceCount, const char* backbufferName, U32 backbufferWidth, U32 backbufferHeight)
	{
		++resourceCount;
		ResourceNames.reserve(resourceCount);
		ResourceInfo.reserve(resourceCount);
		ResourceLifetimes.reserve(resourceCount);
		AddResource(backbufferName, { backbufferWidth, backbufferHeight, 1, { Settings::GetBackbufferFormat() } });
	}

	void FrameBufferDesc::AddResource(std::string&& name, FrameResourceDesc&& info)
	{
		if (std::find(ResourceNames.begin(), ResourceNames.end(), name) != ResourceNames.end())
			throw ZE_RGC_EXCEPT("Frame Buffer already contains resource [" + name + "]!");
		ResourceNames.emplace_back(std::forward<std::string>(name));
		ResourceInfo.emplace_back(std::forward<FrameResourceDesc>(info));
		ResourceLifetimes.emplace_back(std::map<U64, Resource::State>({}));
	}
}
#include "GFX/Resource/FrameBufferDesc.h"
#include "Exception/RenderGraphCompileException.h"

namespace ZE::GFX::Resource
{
	void FrameBufferDesc::Reserve(U64 count) noexcept
	{
		ResourceNames.reserve(count);
		ResourceInfo.reserve(count);
		ResourceLifetimes.reserve(count);
	}

	void FrameBufferDesc::AddResource(std::string&& name, FrameResourceDesc&& info, U64 starLevel)
	{
		if (std::find(ResourceNames.begin(), ResourceNames.end(), name) != ResourceNames.end())
			throw ZE_RGC_EXCEPT("Frame Buffer already contains resource [" + name + "]!");
		ResourceNames.emplace_back(std::forward<std::string>(name));
		ResourceInfo.emplace_back(std::forward<FrameResourceDesc>(info));
		ResourceLifetimes.emplace_back(FrameResourceLifetime(starLevel, {}));
	}
}
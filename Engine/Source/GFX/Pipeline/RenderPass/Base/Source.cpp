#include "GFX/Pipeline/RenderPass/Base/Source.h"

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	bool Source::IsValidName(const std::string& name) noexcept
	{
		return !std::isdigit(name.front()) && std::all_of(name.begin(), name.end(), [](char c) { return std::isalnum(c) || c == '_'; });
	}

	Source::Source(std::string&& sourceName) : name(std::move(sourceName))
	{
		if (name.empty())
			throw ZE_RGC_EXCEPT("Source name empty!");
		if (!IsValidName(name))
			throw ZE_RGC_EXCEPT("Invalid Source name: " + name);
	}
}
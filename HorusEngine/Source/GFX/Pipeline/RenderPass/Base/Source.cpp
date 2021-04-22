#include "GFX/Pipeline/RenderPass/Base/Source.h"

namespace GFX::Pipeline::RenderPass::Base
{
	bool Source::IsValidName(const std::string& name) noexcept
	{
		return !std::isdigit(name.front()) && std::all_of(name.begin(), name.end(), [](char c) { return std::isalnum(c) || c == '_'; });
	}

	Source::Source(std::string&& sourceName) : name(std::move(sourceName))
	{
		if (name.empty())
			throw RGC_EXCEPT("Source name empty!");
		if (!IsValidName(name))
			throw RGC_EXCEPT("Invalid Source name: " + name);
	}
}
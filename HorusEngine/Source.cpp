#include "Source.h"
#include <algorithm>

namespace GFX::Pipeline::RenderPass::Base
{
	bool Source::IsValidName(const std::string& name)
	{
		return !std::isdigit(name.front()) && std::all_of(name.begin(), name.end(), [](char c) { return std::isalnum(c) || c == '_'; });
	}

	Source::Source(const std::string& name) : name(name)
	{
		if (name.empty())
			throw RGC_EXCEPT("Source name empty!");
		if (!IsValidName(name))
			throw RGC_EXCEPT("Invalid Source name: " + name);
	}
}
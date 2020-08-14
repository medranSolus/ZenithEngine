#include "Sink.h"
#include <algorithm>

namespace GFX::Pipeline::RenderPass::Base
{
	bool Sink::IsValidName(const std::string& name)
	{
		return !std::isdigit(name.front()) && std::all_of(name.begin(), name.end(), [](char c) { return std::isalnum(c) || c == '_'; });
	}

	Sink::Sink(const std::string& registeredName) : registeredName(registeredName)
	{
		if (registeredName.empty())
			throw RGC_EXCEPT("Sink name empty!");
		if (!IsValidName(registeredName))
			throw RGC_EXCEPT("Invalid Sink name: " + registeredName);
	}

	void Sink::SetSource(const std::string passName, const std::string& sourceName)
	{
		if (passName.empty())
			throw RGC_EXCEPT("Sink \"" + registeredName + "\" pass name empty!");
		if (passName != "$" && !IsValidName(passName))
			throw RGC_EXCEPT("Invalid Sink \"" + registeredName + "\" pass name: " + passName);
		this->passName = passName;
		if (sourceName.empty())
			throw RGC_EXCEPT("Sink \"" + registeredName + "\" source name empty!");
		if (!IsValidName(sourceName))
			throw RGC_EXCEPT("Invalid Sink \"" + registeredName + "\" source name: " + sourceName);
		this->sourceName = sourceName;
	}
}
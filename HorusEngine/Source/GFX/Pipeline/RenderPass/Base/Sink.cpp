#include "GFX/Pipeline/RenderPass/Base/Sink.h"

namespace GFX::Pipeline::RenderPass::Base
{
	bool Sink::IsValidName(const std::string& name) noexcept
	{
		return !std::isdigit(name.front()) && std::all_of(name.begin(), name.end(), [](char c) { return std::isalnum(c) || c == '_'; });
	}

	Sink::Sink(std::string&& name) : registeredName(std::move(name))
	{
		if (registeredName.empty())
			throw RGC_EXCEPT("Sink name empty!");
		if (!IsValidName(registeredName))
			throw RGC_EXCEPT("Invalid Sink name: " + registeredName);
	}

	std::string Sink::GetPassPathString() const noexcept
	{
		std::string path;
		for (const auto& passName : passPath)
			path += passName + ".";
		path.pop_back();
		return path;
	}

	void Sink::SetSource(std::deque<std::string>&& passPath, std::string&& sourceName)
	{
		if (passPath.empty())
			throw RGC_EXCEPT("Sink \"" + registeredName + "\" pass path empty!");
		for (auto& passName : passPath)
		{
			if (passName.empty())
				throw RGC_EXCEPT("Sink \"" + registeredName + "\" pass name empty!");
			if (passName != "$" && !IsValidName(passName))
				throw RGC_EXCEPT("Invalid Sink \"" + registeredName + "\" pass name: " + passName);
		}
		this->passPath = std::move(passPath);

		if (sourceName.empty())
			throw RGC_EXCEPT("Sink \"" + registeredName + "\" source name empty!");
		if (!IsValidName(sourceName))
			throw RGC_EXCEPT("Invalid Sink \"" + registeredName + "\" source name: " + sourceName);
		this->sourceName = std::move(sourceName);
	}
}
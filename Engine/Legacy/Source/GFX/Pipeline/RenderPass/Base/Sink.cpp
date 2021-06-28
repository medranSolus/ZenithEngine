#include "GFX/Pipeline/RenderPass/Base/Sink.h"

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	bool Sink::IsValidName(const std::string& name) noexcept
	{
		return !std::isdigit(name.front()) && std::all_of(name.begin(), name.end(), [](char c) { return std::isalnum(c) || c == '_'; });
	}

	Sink::Sink(std::string&& name) : registeredName(std::move(name))
	{
		if (registeredName.empty())
			throw ZE_RGC_EXCEPT("Sink name empty!");
		if (!IsValidName(registeredName))
			throw ZE_RGC_EXCEPT("Invalid Sink name: " + registeredName);
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
			throw ZE_RGC_EXCEPT("Sink \"" + registeredName + "\" pass path empty!");
		for (auto& passName : passPath)
		{
			if (passName.empty())
				throw ZE_RGC_EXCEPT("Sink \"" + registeredName + "\" pass name empty!");
			if (passName != "$" && !IsValidName(passName))
				throw ZE_RGC_EXCEPT("Invalid Sink \"" + registeredName + "\" pass name: " + passName);
		}
		this->passPath = std::move(passPath);

		if (sourceName.empty())
			throw ZE_RGC_EXCEPT("Sink \"" + registeredName + "\" source name empty!");
		if (!IsValidName(sourceName))
			throw ZE_RGC_EXCEPT("Invalid Sink \"" + registeredName + "\" source name: " + sourceName);
		this->sourceName = std::move(sourceName);
	}
}
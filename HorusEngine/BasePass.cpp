#include "BasePass.h"
#include "RenderGraphCompileException.h"
#include "Utils.h"

namespace GFX::Pipeline::RenderPass::Base
{
	void BasePass::RegisterSink(std::unique_ptr<Sink> sink)
	{
		for (auto& s : sinks)
			if (s->GetRegisteredName() == sink->GetRegisteredName())
				throw RGC_EXCEPT("Input \"" + sink->GetRegisteredName() + "\" already exists in pass: " + GetName());
		sinks.emplace_back(std::move(sink));
	}

	void BasePass::RegisterSource(std::unique_ptr<Source> source)
	{
		for (auto& s : sources)
			if (s->GetName() == source->GetName())
				throw RGC_EXCEPT("Output \"" + source->GetName() + "\" already exists in pass: " + GetName());
		sources.emplace_back(std::move(source));
	}

	void BasePass::Finalize()
	{
		for (auto& sink : sinks)
			sink->ValidateLink();
		for (auto& source : sources)
			source->ValidateLink();
	}

	void BasePass::SetSinkLinkage(const std::string& registeredName, const std::string& targetName)
	{
		auto& sink = GetSink(registeredName);
		auto source = splitString(registeredName, ".");
		if (source.size() != 2U)
			throw RGC_EXCEPT("Cannot set linkage, Source name in wrong format \"" + registeredName + "\" in pass: " + GetName());
		sink.SetSource(std::move(source.at(0)), std::move(source.at(1)));
	}

	Sink& BasePass::GetSink(const std::string& registeredName) const
	{
		for (auto& sink : sinks)
			if (sink->GetRegisteredName() == registeredName)
				return *sink;
		throw RGC_EXCEPT("Sink \"" + registeredName + "\" not found in pass: " + GetName());
	}

	Source& BasePass::GetSource(const std::string& registeredName) const
	{
		for (auto& source : sources)
			if (source->GetName() == registeredName)
				return *source;
		throw RGC_EXCEPT("Source \"" + registeredName + "\" not found in pass: " + GetName());
	}
}
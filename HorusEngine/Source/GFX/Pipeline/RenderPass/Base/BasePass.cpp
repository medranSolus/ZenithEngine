#include "GFX/Pipeline/RenderPass/Base/BasePass.h"

namespace GFX::Pipeline::RenderPass::Base
{
	void BasePass::RegisterSink(std::unique_ptr<Sink>&& sink)
	{
		for (auto& s : sinks)
			if (s->GetRegisteredName() == sink->GetRegisteredName())
				throw RGC_EXCEPT("Input \"" + sink->GetRegisteredName() + "\" already exists in pass: " + GetName());
		sinks.emplace_back(std::move(sink));
	}

	void BasePass::RegisterSource(std::unique_ptr<Source>&& source)
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
		for (auto& innerPass : GetInnerPasses())
			innerPass->Finalize();
	}

	void BasePass::SetSinkLinkage(const std::string& registeredName, const std::string& targetName)
	{
		auto nameChain = Utils::SplitString(targetName, ".");
		std::string source = std::move(nameChain.back());
		nameChain.pop_back();
		if (nameChain.size() == 0)
			throw RGC_EXCEPT("Cannot set linkage, Source name in wrong format \"" + targetName + "\" in pass: " + GetName());
		GetSink(registeredName).SetSource(std::move(nameChain), std::move(source));
	}

	Sink& BasePass::GetSink(const std::string& registeredName)
	{
		for (auto& sink : sinks)
			if (sink->GetRegisteredName() == registeredName)
				return *sink;
		for (const auto& innerPass : GetInnerPasses())
			for (auto& sink : innerPass->GetSinks())
				if (sink->GetRegisteredName() == registeredName)
					return *sink;
		throw RGC_EXCEPT("Sink \"" + registeredName + "\" not found in pass: " + GetName());
	}

	Source& BasePass::GetSource(const std::string& registeredName)
	{
		for (auto& source : sources)
			if (source->GetName() == registeredName)
				return *source;
		throw RGC_EXCEPT("Source \"" + registeredName + "\" not found in pass: " + GetName());
	}
}
#include "GFX/Pipeline/RenderGraph.h"
#include "GFX/Pipeline/RenderPass/Base/RenderPassesBase.h"
#include "GFX/Pipeline/Resource/RenderTarget.h"

namespace ZE::GFX::Pipeline
{
	RenderGraph::RenderGraph(Graphics& gfx)
		: backbuffer(gfx.GetBackBuffer()), depthStencil(gfx)
	{
		AddGlobalSource(RenderPass::Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));
		AddGlobalSource(RenderPass::Base::SourceDirectBuffer<Resource::RenderTarget>::Make("backbuffer", backbuffer));
		AddGlobalSink(RenderPass::Base::SinkDirectBuffer<Resource::RenderTarget>::Make("backbuffer", backbuffer));
	}

	RenderPass::Base::QueuePass& RenderGraph::GetRenderQueue(const std::string& passName)
	{
		try
		{
			auto nameChain = Utils::SplitString(passName, ".");
			if (nameChain.size() == 1)
			{
				for (auto& pass : passes)
					if (pass->GetName() == passName)
						return dynamic_cast<RenderPass::Base::QueuePass&>(*pass);
			}
			else
			{
				std::string outerName = nameChain.front();
				nameChain.pop_front();
				for (auto& pass : passes)
					if (pass->GetName() == outerName)
						return dynamic_cast<RenderPass::Base::QueuePass&>(pass->GetInnerPass(nameChain));
			}
		}
		catch (std::bad_cast&)
		{
			throw ZE_RGC_EXCEPT("Requested RenderQueue is not a QueuePass: " + passName + "!");
		}
		throw ZE_RGC_EXCEPT("Requested RenderQueue not found: " + passName + "!");
	}

	void RenderGraph::Execute(Graphics& gfx)
	{
		assert(finalized);
		for (auto& pass : passes)
			pass->Execute(gfx);
	}

	void RenderGraph::Reset() noexcept
	{
		assert(finalized);
		for (auto& pass : passes)
			pass->Reset();
	}

	void RenderGraph::LinkSinks(RenderPass::Base::BasePass& pass)
	{
		for (auto& sink : pass.GetSinks())
		{
			auto nameChain = sink->GetPassPath();
			if (nameChain.size() == 0)
				throw ZE_RGC_EXCEPT("Sink \"" + pass.GetName() + "." + sink->GetRegisteredName() + "\" without Source!");
			const std::string sinkSourcePassName = nameChain.front();
			bool sinkOrphaned = true;
			// Global sources
			if (sinkSourcePassName == "$")
			{
				for (auto& source : globalSources)
				{
					if (source->GetName() == sink->GetSourceName())
					{
						sink->Bind(*source);
						sinkOrphaned = false;
						break;
					}
				}
			}
			else if (nameChain.size() > 1)
			{
				nameChain.pop_front();
				sink->Bind(FindPass(sinkSourcePassName).GetInnerPass(nameChain).GetSource(sink->GetSourceName()));
				sinkOrphaned = false;
			}
			else
			{
				for (auto& existingPass : passes)
				{
					if (existingPass->GetName() == sinkSourcePassName)
					{
						sink->Bind(existingPass->GetSource(sink->GetSourceName()));
						sinkOrphaned = false;
						break;
					}
				}
			}
			// Sink not found any source
			if (sinkOrphaned)
				throw ZE_RGC_EXCEPT("Sink \"" + pass.GetName() + "." + sink->GetRegisteredName() +
					"\" not found it's Source \"" + sinkSourcePassName + "." + sink->GetSourceName() + "\"!");
		}
	}

	void RenderGraph::LinkGlobalSinks()
	{
		for (auto& sink : globalSinks)
		{
			auto nameChain = sink->GetPassPath();
			const std::string sinkSourcePassName = nameChain.front();
			bool sinkOrphaned = true;
			if (nameChain.size() == 1)
			{
				for (auto& existingPass : passes)
				{
					if (existingPass->GetName() == sinkSourcePassName)
					{
						sink->Bind(existingPass->GetSource(sink->GetSourceName()));
						sinkOrphaned = false;
						break;
					}
				}
			}
			else
			{
				nameChain.pop_front();
				sink->Bind(FindPass(sinkSourcePassName).GetInnerPass(nameChain).GetSource(sink->GetSourceName()));
				sinkOrphaned = false;
			}
			// Sink not found any source
			if (sinkOrphaned)
				throw ZE_RGC_EXCEPT("Global Sink \"" + sink->GetRegisteredName() +
					"\" not found it's Source \"" + sinkSourcePassName + "." + sink->GetSourceName() + "\"!");
		}
	}

	RenderPass::Base::BasePass& RenderGraph::FindPass(const std::string& name)
	{
		auto nameChain = Utils::SplitString(name, ".");
		if (nameChain.size() == 1)
		{
			for (auto& pass : passes)
				if (pass->GetName() == name)
					return *pass;
		}
		else
		{
			const std::string outerName = nameChain.front();
			nameChain.pop_front();
			for (auto& pass : passes)
				if (pass->GetName() == outerName)
					return pass->GetInnerPass(nameChain);
		}
		throw ZE_RGC_EXCEPT("Pass \"" + name + "\" not present in RenderGraph!");
	}

	void RenderGraph::AppendPass(std::unique_ptr<RenderPass::Base::BasePass>&& pass)
	{
		assert(!finalized);
		for (auto& p : passes)
			if (p->GetName() == pass->GetName())
				throw ZE_RGC_EXCEPT("Pass name already in RenderGraph: " + pass->GetName());
		auto& currentPass = *pass;
		passes.emplace_back(std::move(pass));
		// Link outputs from other passes to this pass inputs
		for (auto& innerPass : currentPass.GetInnerPasses())
			LinkSinks(*innerPass);
		LinkSinks(currentPass);
	}

	void RenderGraph::SetSinkSource(const std::string& sink, const std::string& targetName)
	{
		const auto iter = std::find_if(globalSinks.begin(), globalSinks.end(), [&sink](const std::unique_ptr<RenderPass::Base::Sink>& s)
			{
				return s->GetRegisteredName() == sink;
			});
		if (iter == globalSinks.end())
			throw ZE_RGC_EXCEPT("Global Sink does not exist: " + sink + "!");

		auto nameChain = Utils::SplitString(targetName, ".");
		if (nameChain.size() < 2)
			throw ZE_RGC_EXCEPT("Setting Source in RenderGraph with incorrect format \"" + targetName + "\" for Sink \"" + sink + "\"!");
		std::string source = std::move(nameChain.back());
		nameChain.pop_back();
		(*iter)->SetSource(std::move(nameChain), std::move(source));
	}

	void RenderGraph::Finalize()
	{
		assert(!finalized);
		for (auto& pass : passes)
			pass->Finalize();
		LinkGlobalSinks();
		finalized = true;
	}
}
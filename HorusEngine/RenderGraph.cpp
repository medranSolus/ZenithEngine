#include "RenderGraph.h"
#include "RenderPassesBase.h"
#include "Utils.h"

namespace GFX::Pipeline
{
	RenderGraph::RenderGraph(Graphics& gfx) : backbuffer(gfx.GetBackBuffer())
	{
		depthStencil = std::make_shared<Resource::DepthStencil>(gfx, gfx.GetWidth(), gfx.GetHeight());

		AddGlobalSource(RenderPass::Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));
		AddGlobalSource(RenderPass::Base::SourceDirectBuffer<Resource::RenderTarget>::Make("backbuffer", backbuffer));
		AddGlobalSink(RenderPass::Base::SinkDirectBuffer<Resource::RenderTarget>::Make("backbuffer", backbuffer));
	}

	RenderPass::Base::QueuePass& RenderGraph::GetRenderQueue(const std::string& passName)
	{
		try
		{
			for (auto& pass : passes)
				if (pass->GetName() == passName)
					return dynamic_cast<RenderPass::Base::QueuePass&>(*pass);
		}
		catch (std::bad_cast&)
		{
			throw RGC_EXCEPT("Requested RenderQueue is not a QueuePass \"" + passName + "\"!");
		}
		throw RGC_EXCEPT("Requested RenderQueue not found \"" + passName + "\"!");
	}

	void RenderGraph::Execute(Graphics& gfx) noexcept(!IS_DEBUG)
	{
		assert(finalized);
		for (auto& pass : passes)
			pass->Execute(gfx);
	}

	void RenderGraph::Reset() noexcept(!IS_DEBUG)
	{
		assert(finalized);
		for (auto& pass : passes)
			pass->Reset();
	}

	void RenderGraph::LinkSinks(RenderPass::Base::BasePass& pass)
	{
		for (auto& sink : pass.GetSinks())
		{
			const std::string sinkSourcePassName = sink->GetPassName();
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
				throw RGC_EXCEPT("Sink \"" + sink->GetRegisteredName() + "\" not found its Source \"" + sinkSourcePassName + "." + sink->GetSourceName() + "\"!");
		}
	}

	void RenderGraph::LinkGlobalSinks()
	{
		for (auto& sink : globalSinks)
		{
			const std::string sinkSourcePassName = sink->GetPassName();
			bool sinkOrphaned = true;
			for (auto& existingPass : passes)
			{
				if (existingPass->GetName() == sinkSourcePassName)
				{
					sink->Bind(existingPass->GetSource(sink->GetSourceName()));
					sinkOrphaned = false;
					break;
				}
			}
			// Sink not found any source
			if (sinkOrphaned)
				throw RGC_EXCEPT("Global Sink \"" + sink->GetRegisteredName() + "\" not found its Source \"" + sinkSourcePassName + "." + sink->GetSourceName() + "\"!");
		}
	}

	void RenderGraph::AppendPass(std::unique_ptr<RenderPass::Base::BasePass> pass)
	{
		assert(!finalized);
		for (auto& p : passes)
			if (p->GetName() == pass->GetName())
				throw RGC_EXCEPT("Pass name already in RenderGraph: " + pass->GetName());
		// Link outputs from other passes to this pass inputs
		LinkSinks(*pass);
		passes.emplace_back(std::move(pass));
	}

	void RenderGraph::SetSinkSource(const std::string& sink, const std::string& targetName)
	{
		const auto iter = std::find_if(globalSinks.begin(), globalSinks.end(), [&sink](const std::unique_ptr<RenderPass::Base::Sink>& s)
			{
				return s->GetRegisteredName() == sink;
			});
		if (iter == globalSinks.end())
			throw RGC_EXCEPT("Global Sink does not exist: " + sink + "!");
		auto source = splitString(targetName, ".");
		if (source.size() != 2U)
			throw RGC_EXCEPT("Setting Source in RenderGraph with incorrect format \"" + targetName + "\" for Sink \"" + sink + "\"!");
		(*iter)->SetSource(std::move(source.at(0)), std::move(source.at(1)));
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
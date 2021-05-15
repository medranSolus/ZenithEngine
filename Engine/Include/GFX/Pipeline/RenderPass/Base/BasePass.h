#pragma once
#include "Sink.h"
#include <deque>

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	class BasePass
	{
		std::string name;
		std::vector<std::unique_ptr<Sink>> sinks;
		std::vector<std::unique_ptr<Source>> sources;

	protected:
		void RegisterSink(std::unique_ptr<Sink>&& sink);
		void RegisterSource(std::unique_ptr<Source>&& source);

		BasePass(std::string&& name) noexcept : name(std::move(name)) {}

	public:
		BasePass(BasePass&&) = default;
		BasePass(const BasePass&) = default;
		BasePass& operator=(BasePass&&) = default;
		BasePass& operator=(const BasePass&) = default;
		virtual ~BasePass() = default;

		constexpr const std::string& GetName() const noexcept { return name; }
		constexpr const std::vector<std::unique_ptr<Sink>>& GetSinks() const noexcept { return sinks; }

		virtual constexpr void Reset() noexcept {}
		virtual std::vector<BasePass*> GetInnerPasses() { return {}; }
		virtual BasePass& GetInnerPass(const std::deque<std::string>& nameChain) { throw ZE_RGC_EXCEPT("Pass \"" + name + "\" don't have inner pass named: " + nameChain.front()); }

		virtual void Execute(Graphics& gfx) = 0;
		virtual void Finalize();
		void SetSinkLinkage(const std::string& registeredName, const std::string& targetName);
		Sink& GetSink(const std::string& registeredName);
		Source& GetSource(const std::string& registeredName);
	};
}
#pragma once
#include "Sink.h"

namespace GFX::Pipeline::RenderPass::Base
{
	class BasePass
	{
		std::string name;
		std::vector<std::unique_ptr<Sink>> sinks;
		std::vector<std::unique_ptr<Source>> sources;

	protected:
		void RegisterSink(std::unique_ptr<Sink> sink);
		void RegisterSource(std::unique_ptr<Source> source);

	public:
		inline BasePass(const std::string& name) noexcept : name(name) {}
		BasePass(const BasePass&) = default;
		BasePass& operator=(const BasePass&) = default;
		virtual ~BasePass() = default;

		constexpr const std::string& GetName() const noexcept { return name; }
		constexpr const std::vector<std::unique_ptr<Sink>>& GetSinks() const noexcept { return sinks; }
		inline virtual void Reset() noexcept {}

		virtual void Execute(Graphics& gfx) = 0;
		virtual void Finalize() const;
		void SetSinkLinkage(const std::string& registeredName, const std::string& targetName);
		Sink& GetSink(const std::string& registeredName) const;
		Source& GetSource(const std::string& registeredName) const;
	};
}
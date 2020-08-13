#pragma once
#include "Graphics.h"
#include "Sink.h"
#include "Source.h"

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

		inline virtual void Reset() noexcept {}
		inline const std::string& GetName() const noexcept { return name; }
		inline const std::vector<std::unique_ptr<Sink>>& GetSinks() const noexcept { return sinks; }

		virtual void Execute(Graphics& gfx) noexcept(!IS_DEBUG) = 0;
		virtual void Finalize();
		void SetSinkLinkage(const std::string& registeredName, const std::string& targetName);
		Sink& GetSink(const std::string& registeredName) const;
		Source& GetSource(const std::string& registeredName) const;
	};
}
#pragma once
#include "Source.h"
#include <deque>

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	class Sink
	{
		std::string registeredName;
		std::deque<std::string> passPath;
		std::string sourceName;

		static bool IsValidName(const std::string& name) noexcept;

	protected:
		bool linked = false;

		Sink(std::string&& name);

	public:
		Sink(Sink&&) = default;
		Sink(const Sink&) = default;
		Sink& operator=(Sink&&) = default;
		Sink& operator=(const Sink&) = default;
		virtual ~Sink() = default;

		constexpr const std::string& GetRegisteredName() const noexcept { return registeredName; }
		constexpr const std::deque<std::string>& GetPassPath() const noexcept { return passPath; }
		constexpr const std::string& GetSourceName() const noexcept { return sourceName; }
		constexpr void ValidateLink() const { if (!linked) throw ZE_RGC_EXCEPT("Unlinked Sink \"" + GetRegisteredName() + "\"!"); }

		virtual void Bind(Source& source) = 0;
		std::string GetPassPathString() const noexcept;
		void SetSource(std::deque<std::string>&& passPath, std::string&& sourceName);
	};
}
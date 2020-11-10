#pragma once
#include "Source.h"
#include <deque>

namespace GFX::Pipeline::RenderPass::Base
{
	class Sink
	{
		std::string registeredName;
		std::deque<std::string> passPath;
		std::string sourceName;

		static inline bool IsValidName(const std::string& name) noexcept;

	protected:
		bool linked = false;

		Sink(const std::string& registeredName);

	public:
		virtual ~Sink() = default;

		constexpr const std::string& GetRegisteredName() const noexcept { return registeredName; }
		constexpr const std::deque<std::string>& GetPassPath() const noexcept { return passPath; }
		constexpr const std::string& GetSourceName() const noexcept { return sourceName; }
		constexpr void ValidateLink() const { if (!linked) throw RGC_EXCEPT("Unlinked Sink \"" + GetRegisteredName() + "\"!"); }

		virtual void Bind(Source& source) = 0;
		std::string GetPassPathString() const noexcept;
		void SetSource(const std::deque<std::string>& passPath, const std::string& sourceName);
	};
}
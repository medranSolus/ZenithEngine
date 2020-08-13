#pragma once
#include "Source.h"

namespace GFX::Pipeline::RenderPass::Base
{
	class Sink
	{
		std::string registeredName;
		std::string passName;
		std::string sourceName;

		static bool IsValidName(const std::string& name);

	protected:
		Sink(const std::string& registeredName);

	public:
		virtual ~Sink() = default;

		inline const std::string& GetRegisteredName() const noexcept { return registeredName; }
		inline const std::string& GetPassName() const noexcept { return passName; }
		inline const std::string& GetSourceName() const noexcept { return sourceName; }

		virtual void Bind(Source& source) = 0;
		virtual void ValidateLink() const = 0;
		void SetSource(const std::string passName, const std::string& sourceName);
	};
}
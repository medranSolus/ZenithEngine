#pragma once
#include "BasicException.h"

#define RGC_EXCEPT(msg) Exception::RenderGraphCompileException(__LINE__, __FILE__, msg)

namespace Exception
{
	class RenderGraphCompileException : public BasicException
	{
		std::string message;

	public:
		inline RenderGraphCompileException(unsigned int line, const char* file, const std::string& message) noexcept
			: BasicException(line, file), message(message) {}
		RenderGraphCompileException(const RenderGraphCompileException&) = default;
		RenderGraphCompileException& operator=(const RenderGraphCompileException&) = default;
		virtual ~RenderGraphCompileException() = default;

		inline const char* GetType() const noexcept override { return "Render Graph Compile Exception"; }
		constexpr const std::string& GetMessage() const noexcept { return message; }

		const char* what() const noexcept override;
	};
}
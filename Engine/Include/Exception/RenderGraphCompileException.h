#pragma once
#include "Exception/BasicException.h"

namespace ZE::Exception
{
	class RenderGraphCompileException : public BasicException
	{
		std::string message;

	public:
		RenderGraphCompileException(U32 line, const char* file, std::string&& message) noexcept
			: BasicException(line, file), message(std::move(message)) {}
		RenderGraphCompileException(const RenderGraphCompileException&) = default;
		RenderGraphCompileException& operator=(const RenderGraphCompileException&) = default;
		virtual ~RenderGraphCompileException() = default;

		constexpr const char* GetType() const noexcept override { return "Render Graph Compile Exception"; }
		constexpr const std::string& GetMessage() const noexcept { return message; }

		const char* what() const noexcept override;
	};
}

#define ZE_RGC_EXCEPT(msg) ZE::Exception::RenderGraphCompileException(__LINE__, __FILE__, msg)
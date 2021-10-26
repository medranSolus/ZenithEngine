#pragma once
#include "Exception/BasicException.h"

namespace ZE::Exception
{
	// Exception thrown while creating render graph
	class RenderGraphCompileException : public virtual BasicException
	{
		std::string message;

	public:
		RenderGraphCompileException(U32 line, const char* file, std::string&& message) noexcept
			: BasicException(line, file), message(std::move(message)) {}
		RenderGraphCompileException(RenderGraphCompileException&&) = default;
		RenderGraphCompileException(const RenderGraphCompileException&) = default;
		RenderGraphCompileException& operator=(RenderGraphCompileException&&) = default;
		RenderGraphCompileException& operator=(const RenderGraphCompileException&) = default;
		virtual ~RenderGraphCompileException() = default;

		constexpr const std::string& GetMessage() const noexcept { return message; }
		constexpr const char* GetType() const noexcept override { return "Render Graph Compile Exception"; }

		const char* what() const noexcept override;
	};
}

#define ZE_RGC_EXCEPT(msg) ZE::Exception::RenderGraphCompileException(__LINE__, __FILENAME__, msg)
#pragma once
#include "Exception/BasicException.h"

namespace Exception
{
	class ModelException : public BasicException
	{
		std::string error;

	public:
		ModelException(U32 line, const char* file, std::string&& error) noexcept
			: BasicException(line, file), error(std::move(error)) {}
		ModelException(ModelException&&) = default;
		ModelException(const ModelException&) = default;
		ModelException& operator=(ModelException&&) = default;
		ModelException& operator=(const ModelException&) = default;
		virtual ~ModelException() = default;

		constexpr const std::string& GetErrorString() const noexcept { return error; }
		constexpr const char* GetType() const noexcept override { return "Model Exception"; }

		const char* what() const noexcept override;
	};
}

#define MDL_EXCEPT(msg) Exception::ModelException(__LINE__, __FILE__, msg)
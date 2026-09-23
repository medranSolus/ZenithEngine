#pragma once
#include "BasicTypes.h"
#include <string>

namespace ZE::IO
{
	// Facility to get information about encountering EOF with async file operations
	class AsyncEofResult : public std::error_category
	{
	protected:
		AsyncEofResult() = default;

	public:
		ZE_CLASS_MOVE(AsyncEofResult);
		virtual ~AsyncEofResult() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static AsyncEofResult CATEGORY; return CATEGORY; }
		static Status Make(U32 bytes) noexcept { return { static_cast<int>(bytes), GetCategory() }; }

		static bool IsEOF(Status code) noexcept { return code.category() == GetCategory(); }
		static U32 GetRealBytes(Status code) noexcept { ZE_ASSERT(IsEOF(code), "Code is not EOF status!"); return static_cast<U32>(code.value()); }

		const char* name() const noexcept override { return "Async EOF Result"; }
		std::string message(int condition) const override { return std::to_string(static_cast<U32>(condition)); }
	};
}
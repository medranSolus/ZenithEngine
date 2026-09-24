#pragma once
#include "BasicTypes.h"
#include <string>

namespace ZE::IO
{
	// Facility to get information about encountering EOF with file operations
	class EofResult : public std::error_category
	{
	protected:
		EofResult() = default;

	public:
		ZE_CLASS_MOVE(EofResult);
		virtual ~EofResult() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static EofResult CATEGORY; return CATEGORY; }
		static Status Make(U32 bytes) noexcept { return { static_cast<int>(bytes == 0 ? UINT32_MAX : bytes), GetCategory() }; }

		static bool IsEOF(Status code) noexcept { return code.category() == GetCategory(); }
		static U32 GetRealBytes(Status code) noexcept { ZE_ASSERT(IsEOF(code), "Code is not EOF status!"); U32 val = static_cast<U32>(code.value()); return val == UINT32_MAX ? 0 : val; }

		const char* name() const noexcept override { return "File EOF Result"; }
		std::string message(int condition) const override { return std::to_string(static_cast<U32>(condition)); }
	};
}
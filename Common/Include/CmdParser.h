#pragma once
#include "Types.h"
#include <unordered_map>
#include <string_view>
#include <vector>

namespace ZE
{
	// Parser of command line params
	class CmdParser final
	{
		std::unordered_map<std::string, bool> options;
		std::unordered_map<std::string, U32> numbers;
		std::unordered_map<std::string, std::string_view> strings;

		bool ParamPresent(std::string_view name) const noexcept;

	public:
		CmdParser() = default;
		ZE_CLASS_DELETE(CmdParser);
		~CmdParser() = default;

		// Adds on/off parameter
		void AddOption(std::string_view name, bool defValue = false) noexcept { ZE_ASSERT(!ParamPresent(name), "Given name has been already used!"); options.emplace(name, defValue); }
		// Adds retrieveable number
		void AddNumber(std::string_view name, U32 defValue = 0) noexcept { ZE_ASSERT(!ParamPresent(name), "Given name has been already used!"); numbers.emplace(name, defValue); }
		// Adds string parameter
		void AddString(std::string_view name, std::string_view defValue = "") noexcept { ZE_ASSERT(!ParamPresent(name), "Given name has been already used!"); strings.emplace(name, defValue); }

		void Parse(std::string_view clParams) noexcept;
		bool GetOption(std::string_view name) const noexcept;
		U32 GetNumber(std::string_view name) const noexcept;
		std::string_view GetString(std::string_view name) const noexcept;
	};
}
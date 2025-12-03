#pragma once
#include "Types.h"
#include <deque>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ZE
{
	// Parser of command line params
	class CmdParser final
	{
		enum ParamType : U8
		{
			Unknown,
			Option,
			Number,
			Float,
			String
		};

		std::unordered_map<std::string, bool> options;
		std::unordered_map<std::string, U32> numbers;
		std::unordered_map<std::string, float> floats;
		std::unordered_map<std::string, std::string_view> strings;
		std::unordered_map<char, std::pair<ParamType, std::string_view>> shortNames;

		bool ParamPresent(std::string_view name) const noexcept;
		void AddShortName(char shortName, ParamType type, std::string_view name) noexcept;
		void Parse(const std::deque<std::string_view>& params) noexcept;

	public:
		CmdParser() noexcept { AddOption("help", 'h'); }
		ZE_CLASS_DELETE(CmdParser);
		~CmdParser() = default;

		// Adds on/off parameter
		void AddOption(std::string_view name, char shortName = ' ') noexcept;
		// Adds retrieveable number
		void AddNumber(std::string_view name, U32 defValue = 0, char shortName = ' ') noexcept;
		// Adds floating-point number
		void AddFloat(std::string_view name, float defValue = 0.0f, char shortName = ' ') noexcept;
		// Adds string parameter
		void AddString(std::string_view name, std::string_view defValue = "", char shortName = ' ') noexcept;

		void Parse(int argc, char* argv[]) noexcept;
		void Parse(std::string_view clParams) noexcept;
		bool GetOption(std::string_view name) const noexcept;
		U32 GetNumber(std::string_view name) const noexcept;
		float GetFloat(std::string_view name) const noexcept;
		std::string_view GetString(std::string_view name) const noexcept;
	};
}
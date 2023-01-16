#include "CmdParser.h"
#include "Utils.h"

namespace ZE
{
	bool CmdParser::ParamPresent(std::string_view name) const noexcept
	{
		for (const auto& x : options)
			if (!x.first.compare(name))
				return true;
		for (const auto& x : numbers)
			if (!x.first.compare(name))
				return true;
		for (const auto& x : strings)
			if (!x.first.compare(name))
				return true;

		return false;
	}

	void CmdParser::Parse(std::string_view clParams) noexcept
	{
		if (clParams.empty())
			return;

		std::deque<std::string_view> clParameters = Utils::SplitString(clParams, "/");
		for (std::string_view clParam : clParameters)
		{
			bool notFound = true;
			for (auto& option : options)
			{
				if (!option.first.compare(clParam))
				{
					option.second = true;
					notFound = false;
					break;
				}
			}
			if (notFound)
			{
				const std::deque<std::string_view> valuePair = Utils::SplitString(clParam, "=");
				if (valuePair.size() != 2)
				{
					ZE_FAIL("Incorrectly formulated parameter! Should be in form: /name=value");
					continue;
				}
				for (auto& number : numbers)
				{
					if (!number.first.compare(valuePair.front()))
					{
						std::from_chars(valuePair.back().data(), valuePair.back().data() + valuePair.back().size(), number.second);
						notFound = false;
						break;
					}
				}
				if (notFound)
				{
					for (auto& str : strings)
					{
						if (!str.first.compare(valuePair.front()))
						{
							str.second = valuePair.back();
							break;
						}
					}
				}
			}
		}
	}

	bool CmdParser::GetOption(std::string_view name) const noexcept
	{
		ZE_ASSERT(ParamPresent(name), "Unknown parameter!");
		for (const auto& x : options)
			if (!x.first.compare(name))
				return x.second;
		return false;
	}

	U32 CmdParser::GetNumber(std::string_view name) const noexcept
	{
		ZE_ASSERT(ParamPresent(name), "Unknown parameter!");
		for (const auto& x : numbers)
			if (!x.first.compare(name))
				return x.second;
		return 0;
	}

	std::string_view CmdParser::GetString(std::string_view name) const noexcept
	{
		ZE_ASSERT(ParamPresent(name), "Unknown parameter!");
		for (const auto& x : strings)
			if (!x.first.compare(name))
				return x.second;
		return "";
	}
}
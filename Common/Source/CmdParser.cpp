#include "CmdParser.h"
#include "Utils.h"

namespace ZE
{
	bool CmdParser::ParamPresent(std::string_view name) const noexcept
	{
		std::string key(name);
		if (options.contains(key))
			return true;
		if (numbers.contains(key))
			return true;
		if (strings.contains(key))
			return true;
		return false;
	}

	void CmdParser::AddShortName(char shortName, ParamType type, std::string_view name) noexcept
	{
		if (shortName != ' ')
		{
			ZE_ASSERT(!shortNames.contains(shortName), "Short parameter name already registered!");
			shortNames.emplace(shortName, std::make_pair(type, name));
		}
	}

	void CmdParser::Parse(const std::deque<std::string_view>& params) noexcept
	{
		for (U64 i = 0; i < params.size(); ++i)
		{
			std::string_view clParam = params.at(i);
			if (clParam.size() < 2 || clParam.front() != '-')
			{
				Logger::Error("Incorrectly formulated parameter \" " + std::string(clParam) + " \"! All parameters should start with '-'");
				continue;
			}

			std::string_view param = "";
			ParamType type = ParamType::Unknown;
			bool handled = false;
			// Check for short names
			if (clParam.at(1) != '-')
			{
				if (clParam.size() == 2)
				{
					if (shortNames.contains(clParam.at(1)))
					{
						const auto& paramDesc = shortNames.at(clParam.at(1));
						type = paramDesc.first;
						param = paramDesc.second;
					}
					else
					{
						Logger::Error("Unknown short parameter name: -" + std::string(1, clParam.at(1)));
						continue;
					}
				}
				else
				{
					handled = true;
					for (U64 j = 1; j < param.size(); ++j)
					{
						char shortName = clParam.at(j);
						if (shortNames.contains(shortName))
						{
							const auto& paramDesc = shortNames.at(shortName);
							if (paramDesc.first == ParamType::Option)
								options.at(std::string(paramDesc.second)) = true;
							else
								Logger::Error("Short parameter name: -" + std::string(1, shortName) + " is not an option!");
						}
						else
							Logger::Error("Unknown short parameter name: -" + std::string(1, shortName));
					}
				}
			}
			else
				param = clParam.substr(2, clParam.size() - 2);

			if (!handled)
			{
				switch (type)
				{
				default:
					ZE_ENUM_UNHANDLED();
				case ZE::CmdParser::Unknown:
				{
					break;
				}
				case ZE::CmdParser::Option:
				{
					options.at(std::string(param)) = true;
					break;
				}
				case ZE::CmdParser::Number:
				{
					if (++i >= params.size())
					{
						Logger::Error("Parameter list too short, missing value for parameter \"" + std::string(param) + "\"!");
						return;
					}
					if (params.at(i).front() == '-')
					{
						Logger::Error("Expected number value for parameter \"" + std::string(param) + "\", but got parameter \"" + std::string(params.at(i)) + "\"!");
						--i;
					}
					else
						std::from_chars(params.at(i).data(), params.at(i).data() + params.at(i).size(), numbers.at(std::string(param)));
					break;
				}
				case ZE::CmdParser::String:
				{
					if (++i >= params.size())
					{
						Logger::Error("Parameter list too short, missing value for parameter \"" + std::string(param) + "\"!");
						return;
					}
					if (params.at(i).front() == '-')
					{
						Logger::Error("Expected string value for parameter \"" + std::string(param) + "\", but got parameter \"" + std::string(params.at(i)) + "\"!");
						break;
					}
					else
						strings.at(std::string(param)) = params.at(i);
					break;
				}
				}
			}
		}

		if (GetOption("help"))
		{
			Logger::InfoNoFile("Available command line parameters:");
			for (const auto& val : options)
				Logger::InfoNoFile("  --" + val.first);
			for (const auto& val : numbers)
				Logger::InfoNoFile("  --" + val.first + " <NUMBER>");
			for (const auto& val : strings)
				Logger::InfoNoFile("  --" + val.first + " <STRING>");

			Logger::InfoNoFile("Short names for parameters:");
			for (const auto& val : shortNames)
				Logger::InfoNoFile("  -" + std::string(1, val.first) + " = --" + std::string(val.second.second));
		}
	}

	void CmdParser::AddOption(std::string_view name, char shortName) noexcept
	{
		ZE_ASSERT(!ParamPresent(name), "Given name has been already used!");
		options.emplace(name, false);
		AddShortName(shortName, ParamType::Option, name);
	}

	void CmdParser::AddNumber(std::string_view name, U32 defValue, char shortName) noexcept
	{
		ZE_ASSERT(!ParamPresent(name), "Given name has been already used!");
		numbers.emplace(name, defValue);
		AddShortName(shortName, ParamType::Number, name);
	}

	void CmdParser::AddString(std::string_view name, std::string_view defValue, char shortName) noexcept
	{
		ZE_ASSERT(!ParamPresent(name), "Given name has been already used!");
		strings.emplace(name, defValue);
		AddShortName(shortName, ParamType::String, name);
	}

	void CmdParser::Parse(int argc, char* argv[]) noexcept
	{
		if (argc < 2)
			return;

		std::deque<std::string_view> argvParams;
		for (int i = 1; i < argc; ++i)
			argvParams.emplace_back(argv[i]);
		Parse(argvParams);
	}

	void CmdParser::Parse(std::string_view clParams) noexcept
	{
		if (clParams.empty())
			return;
		Parse(Utils::SplitString(clParams, " "));
	}

	bool CmdParser::GetOption(std::string_view name) const noexcept
	{
		std::string key(name);
		if (options.contains(key))
			return options.at(key);
		ZE_FAIL("Unknown option: " + key);
		return false;
	}

	U32 CmdParser::GetNumber(std::string_view name) const noexcept
	{
		std::string key(name);
		if (numbers.contains(key))
			return numbers.at(key);
		ZE_FAIL("Unknown number: " + key);
		return 0;
	}

	std::string_view CmdParser::GetString(std::string_view name) const noexcept
	{
		std::string key(name);
		if (strings.contains(key))
			return strings.at(key);
		ZE_FAIL("Unknown parameter: " + key);
		return "";
	}
}
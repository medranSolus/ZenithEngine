#pragma once
#include "json.hpp"
#include <string>
#include <vector>

namespace json = nlohmann;

class ScriptProcess
{
	enum OutCode : int
	{
		NoInput = 1,
		Good = 0,
		UnknownError = -1,
		NotEnoughParams = -2,
		TooMuchParams = -3,
		WrongOption = -4,
		CannotOpenFile = -5,
		InvalidJsonCommand = -6
	};

	static OutCode ProcessJsonCommand(const json::json& command);
	static OutCode ProcessJson(const std::string& jsonFile);

public:
	ScriptProcess() = delete;

	static int Run(const std::vector<std::string>& params);
};
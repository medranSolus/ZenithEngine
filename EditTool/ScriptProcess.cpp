#include "ScriptProcess.h"
#include "TextureEdit.h"
#include "Logger.h"
#include <fstream>

ScriptProcess::OutCode ScriptProcess::ProcessJsonCommand(const json::json& command)
{
	const std::string commandName = command["command"].get<std::string>();
	const auto params = command["params"];
	if (commandName == "no-alpha")
	{
		const auto it = params.find("destination");
		const std::string source = params["source"].get<std::string>();
		const std::string destination = it != params.end() ? it->get<std::string>() : source;
		TextureEdit::NoAlpha(source, destination);
	}
	else
	{
		Logger::Error("Unknown JSON command! Command: " + commandName);
		return OutCode::InvalidJsonCommand;
	}
	return OutCode::Good;
}

ScriptProcess::OutCode ScriptProcess::ProcessJson(const std::string& jsonFile)
{
	std::ifstream fin(jsonFile);
	if (!fin.good())
	{
		fin.close();
		Logger::Error("Cannot open file: " + jsonFile);
		return OutCode::CannotOpenFile;
	}
	json::json jsonArray;
	fin >> jsonArray;
	if (jsonArray.is_array())
	{
		for (const auto& json : jsonArray)
		{
			OutCode code = ProcessJsonCommand(json);
			if (code != OutCode::Good)
				return code;
		}
		return OutCode::Good;
	}
	else
		return ProcessJsonCommand(jsonArray);
}

int ScriptProcess::Run(const std::vector<std::string>& params)
{
	if (params.size())
	{
		if (params.front() == "--json" || params.front() == "-j")
		{
			if (params.size() >= 2)
			{
				OutCode code;
				for (size_t i = 1; i < params.size(); ++i)
				{
					code = ProcessJson(params.at(i));
					if (code != OutCode::Good)
					{
						Logger::Error("Error processing JSON!");
						return code;
					}
				}
			}
			else
			{
				Logger::Error("No JSON files in input!");
				return OutCode::NotEnoughParams;
			}
		}
		else
		{
			Logger::Error("Invalid option!");
			return OutCode::WrongOption;
		}
		return OutCode::Good;
	}
	return OutCode::NoInput;
}
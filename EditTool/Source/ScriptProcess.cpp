#include "ScriptProcess.h"
#include "TextureEdit.h"

ScriptProcess::OutCode ScriptProcess::GetSrcDest(std::string& source, std::string& destination, std::deque<std::string>& params) noexcept
{
	params.pop_front();
	if (params.size() == 0 || params.front().at(0) == '-')
	{
		ZE::Logger::Error("No files in input!");
		return OutCode::NotEnoughParams;
	}
	source = params.front();
	params.pop_front();
	if (params.size() && params.front().at(0) != '-')
	{
		destination = params.front();
		params.pop_front();
	}
	else
		destination = source;
	return OutCode::Good;
}

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
	else if (commandName == "flip-y")
	{
		const auto it = params.find("destination");
		const std::string source = params["source"].get<std::string>();
		const std::string destination = it != params.end() ? it->get<std::string>() : source;
		TextureEdit::FlipY(source, destination);
	}
	else
	{
		ZE::Logger::Error("Unknown JSON command! Command: " + commandName);
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
		ZE::Logger::Error("Cannot open file: " + jsonFile);
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

int ScriptProcess::Run(std::deque<std::string>& params)
{
	if (params.size() == 0)
		return OutCode::NoInput;
	while (params.size())
	{
		if (params.front() == "--json" || params.front() == "-j")
		{
			params.pop_front();
			if (params.size() == 0)
			{
				ZE::Logger::Error("No JSON files in input!");
				return OutCode::NotEnoughParams;
			}
			size_t i = 0;
			for (; i < params.size(); ++i)
			{
				if (params.at(i).at(0) == '-')
					break;
				OutCode code = ProcessJson(params.at(i));
				if (code != OutCode::Good)
				{
					ZE::Logger::Error("Error processing JSON!");
					return code;
				}
			}
			params.erase(params.begin(), params.begin() + i);
		}
		else if (params.front() == "--no-alpha")
		{
			std::string source, destination;
			OutCode code = GetSrcDest(source, destination, params);
			if (code != OutCode::Good)
				return code;
			TextureEdit::NoAlpha(source, destination);
		}
		else if (params.front() == "--flip-y")
		{
			std::string source, destination;
			OutCode code = GetSrcDest(source, destination, params);
			if (code != OutCode::Good)
				return code;
			TextureEdit::FlipY(source, destination);
		}
		else
		{
			ZE::Logger::Error("Invalid option!");
			return OutCode::WrongOption;
		}
	}
	return OutCode::Good;
}
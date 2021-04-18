#include "Logger.h"
#include <iostream>
#include <fstream>

void Logger::Log(Level type, const std::string& log, bool noFile)
{
	std::string banner;
	bool error = false;
	switch (type)
	{
	case Logger::Level::Info:
	{
		banner = "[INFO] ";
		break;
	}
	case Logger::Level::Warning:
	{
		banner = "[WARNING] ";
		break;
	}
	case Logger::Level::Error:
	{
		banner = "[ERROR] ";
		error = true;
		break;
	}
	}
	if (noFile)
	{
		std::ofstream fout;
		if (firstUse)
		{
			firstUse = false;
			fout.open("log.txt", std::ofstream::trunc);
		}
		else
			fout.open("log.txt", std::ofstream::app);
		if (!fout.good())
		{
			std::cerr << "[ERROR] Cannot open log file! Inner log:\n\t";
			error = true;
		}
		else
		{
			fout << banner << log << std::endl;
			fout.close();
		}
	}
	(error ? std::cerr : std::cout) << banner << log << std::endl;
}
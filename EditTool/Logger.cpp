#include "Logger.h"
#include <iostream>
#include <fstream>

bool Logger::firstUse = true;

void Logger::Log(Level type, const std::string& log, bool noFile)
{
	std::string banner;
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
			std::cout << "[ERROR] Cannot open log file! Inner log:\n\t";
			return;
		}
		else
		{
			fout << banner << log << std::endl;
			fout.close();
		}
	}
	std::cout << banner << log << std::endl;
}
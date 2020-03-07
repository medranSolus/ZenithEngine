#include "Logger.h"
#include <fstream>

bool Logger::firstUse = true;

void Logger::Log(Level type, const std::string& log)
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
		return;
	switch (type)
	{
	case Logger::Level::Info:
	{
		fout << "[INFO] ";
		break;
	}
	case Logger::Level::Warning:
	{
		fout << "[WARNING] ";
		break;
	}
	case Logger::Level::Error:
	{
		fout << "[ERROR] ";
		break;
	}
	}
	fout << log;
	fout.close();
}

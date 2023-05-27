#include "Logger.h"
#include <iostream>

namespace ZE
{
	void Logger::Log(Level type, const std::string& log, bool flush, bool logToFile)
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
		if (logToFile)
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
				if (flush)
					fout << std::flush;
				fout.close();
			}
		}
		(error ? std::cerr : std::cout) << banner << log << std::endl;
		if (flush)
			(error ? std::cerr : std::cout) << std::flush;
	}
}
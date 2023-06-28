#include "Logger.h"
#include <iostream>

namespace ZE
{
	void Logger::Log(Level type, const std::string& log, bool flush, bool logToFile)
	{
		std::string_view banner;
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
		auto writeLog = [&](std::ostream& out)
		{
			out << banner << log << std::endl;
			if (flush)
				out << std::flush;
		};

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
				writeLog(fout);
				fout.close();
			}
		}
		writeLog(error ? std::cerr : std::cout);
	}
}
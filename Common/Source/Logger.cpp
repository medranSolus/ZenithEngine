#include "Logger.h"
#include <iostream>

namespace ZE
{
	void Logger::Log(Level type, const std::string& log, bool flush, bool newLine, bool logToFile) noexcept
	{
		std::string_view banner;
		bool error = false;
		switch (type)
		{
		case Logger::Level::Info:
		{
			banner = "> [INFO] ";
			break;
		}
		case Logger::Level::Warning:
		{
			banner = "> [WARNING] ";
			break;
		}
		default:
			ZE_ENUM_UNHANDLED();
		case Logger::Level::Error:
		{
			banner = "> [ERROR] ";
			error = true;
			break;
		}
		}
		auto writeLog = [&](std::ostream& out)
			{
				out << '<' << Utils::GetCurrentTimestamp() << banner << log;
				if (newLine)
					out << std::endl;
				if (flush)
					out << std::flush;
			};

		bool fileError = false;
		if (logToFile)
		{
			LockGuardRW lock(fileMutex);
			if (CreateLogDir(true))
			{
				std::ofstream fout;
				if (firstUse)
				{
					firstUse = false;
					fout.open(LOG_FILE, std::ofstream::trunc);
				}
				else
					fout.open(LOG_FILE, std::ofstream::app);

				if (fout.good())
				{
					writeLog(fout);
					fout.close();
				}
				else
					fileError = true;
			}
			else
				fileError = true;
		}

		LockGuardRW lock(consoleMutex);
		if (fileError)
		{
			std::cerr << '<' << Utils::GetCurrentTimestamp() << "> [ERROR] Cannot open log file \"" << LOG_FILE << "\"! Inner log:\n\t";
			error = true;
		}
		writeLog(error ? std::cerr : std::cout);
	}

	bool Logger::CreateLogDir(bool noLock) noexcept
	{
		if (!std::filesystem::exists(LOG_DIR))
		{
			LockGuardRW lock(fileMutex, !noLock);
			return !std::filesystem::create_directories(LOG_DIR);
		}
		return true;
	}
}
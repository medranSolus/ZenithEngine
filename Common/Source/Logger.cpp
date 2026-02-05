#include "Logger.h"
#include <iomanip>

namespace ZE
{
	void Logger::WriteHeader(std::ostream& out, Level type) noexcept
	{
		out << '<' << Utils::GetCurrentTimestamp();
		switch (type)
		{
		case Level::Info:
		{
			out << "> [INFO]";
			break;
		}
		case Level::Warning:
		{
			out << "> [WARNING]";
			break;
		}
		default:
			ZE_ENUM_UNHANDLED();
		case Level::Error:
		{
			out << "> [ERROR]";
			break;
		}
		case Level::Critical:
		{
			out << "> [CRITICAL]";
			break;
		}
		}
	}

	void Logger::LogToFile(std::function<void(std::ostream&)> writeLog) noexcept
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
				Log(Level::Error, "Cannot open log file \"Logs/log.txt\" for saving following log entry!", false, true, false);
		}
	}

	void Logger::Log(Level type, std::string_view log, bool flush, bool newLine, bool logToFile) noexcept
	{
		auto writeLog = [&](std::ostream& out)
			{
				WriteHeader(out, type);
				out << '\t' << log;
				if (newLine)
					out << std::endl;
				if (flush)
					out << std::flush;
			};

		if (logToFile)
			LogToFile(writeLog);

		LockGuardRW lock(consoleMutex);
		writeLog(type == Level::Error || type == Level::Critical ? std::cerr : std::cout);
	}

	void Logger::LogStatusCode(Level type, const std::error_code& code, const std::string& msg, U32 line, const char* file, bool logToFile) noexcept
	{
		auto writeLog = [&](std::ostream& out)
			{
				WriteHeader(out, type);

				out << "\t[CATEGORY] " << code.category().name() << std::endl << std::setfill(' ')
					<< std::setw(43) << "[LOCATION] " << file << '@' << line << std::endl
					<< std::setw(38) << "[INFO]" << std::setw(5) << ' ' << code.value() << ": " << code.message() << std::endl
					<< std::setw(43) << "[DETAILED] " << msg << std::endl;
			};

		if (logToFile)
			LogToFile(writeLog);

		LockGuardRW lock(consoleMutex);
		writeLog(type == Level::Error || type == Level::Critical ? std::cerr : std::cout);
	}

	bool Logger::CreateLogDir(bool noLock) noexcept
	{
		std::error_code code = {};
		bool exists = std::filesystem::exists(LOG_DIR, code);
		if (code)
		{
#if !_ZE_MODE_RELEASE
			LogStatusCode(Level::Error, code, "Failed to check for log directory existence!", __LINE__, __FILENAME__, false);
#endif
			return false;
		}
		if (!exists)
		{
			LockGuardRW lock(fileMutex, !noLock);
			// Not checking return value since Windows always reports it as false, no matter if directory got created or not
			std::filesystem::create_directories(LOG_DIR, code);
			if (code)
			{
#if !_ZE_MODE_RELEASE
				LogStatusCode(Level::Error, code, "Failed to create log directory!", __LINE__, __FILENAME__, false);
#endif
				return false;
			}
			exists = std::filesystem::exists(LOG_DIR, code);
			if (code)
			{
#if !_ZE_MODE_RELEASE
				LogStatusCode(Level::Error, code, "Failed to check for log directory existence after creating it!", __LINE__, __FILENAME__, false);
#endif
				return false;
			}
		}
		return exists;
	}
}
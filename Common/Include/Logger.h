#pragma once
#include "BasicTypes.h"
#include <functional>
#include <iostream>
#include <shared_mutex>
#include <string>

namespace ZE
{
	// Static logger service
	class Logger final
	{
	public:
		static constexpr const char* LOG_DIR = "./Logs/";
		static constexpr const wchar_t* LOG_DIR_W = L"./Logs/";

	private:
		enum class Level : U8 { Info, Warning, Error, Critical };

		static constexpr const char* LOG_FILE = "./Logs/log.txt";

		static inline bool firstUse = true;
		static inline std::shared_mutex consoleMutex;
		static inline std::shared_mutex fileMutex;

		static void WriteHeader(std::ostream& out, Level type) noexcept;
		static void LogToFile(std::function<void(std::ostream&)> writeLog) noexcept;
		static void Log(Level type, const std::string& log, bool flush, bool newLine = true, bool logToFile = true) noexcept;
		static void LogStatusCode(Level type, const std::error_code& code, const std::string& msg, U32 line, const char* file, bool logToFile = true) noexcept;

	public:
		Logger() = delete;

		static void InfoNoFile(const std::string& info, bool flush = false, bool newLine = true) noexcept { Log(Level::Info, info, flush, newLine, false); }
		static void Info(const std::string& info, bool flush = false, bool newLine = true) noexcept { Log(Level::Info, info, flush, newLine); }
		static void Warning(const std::string& warning, bool flush = false, bool newLine = true) noexcept { Log(Level::Warning, warning, flush, newLine); }
		static void Error(const std::string& error, bool flush = false, bool newLine = true) noexcept { Log(Level::Error, error, flush, newLine); }
		static void Critical(const std::string& error, bool flush = false, bool newLine = true) noexcept { Log(Level::Critical, error, flush, newLine); }

		static void CodeInfo(const std::error_code& code, const std::string& msg, U32 line, const char* file) noexcept { LogStatusCode(Level::Info, code, msg, line, file); }
		static void CodeWarning(const std::error_code& code, const std::string& msg, U32 line, const char* file) noexcept { LogStatusCode(Level::Warning, code, msg, line, file); }
		static void CodeError(const std::error_code& code, const std::string& msg, U32 line, const char* file) noexcept { LogStatusCode(Level::Error, code, msg, line, file); }
		static void CodeCritical(const std::error_code& code, const std::string& msg, U32 line, const char* file) noexcept { LogStatusCode(Level::Critical, code, msg, line, file); }

		static bool CreateLogDir(bool noLock = false) noexcept;
	};
}
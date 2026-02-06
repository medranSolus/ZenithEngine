#pragma once
#include "BasicTypes.h"
#include <functional>
#include <iostream>
#include <shared_mutex>
#include <string_view>

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
		static void Log(Level type, std::string_view log, bool flush, bool newLine = true, bool logToFile = true) noexcept;
		static void LogStatusCode(Level type, const Status& code, std::string_view msg, U32 line, const char* file, bool logToFile = true) noexcept;

	public:
		Logger() = delete;

		static void InfoNoFile(std::string_view info, bool flush = false, bool newLine = true) noexcept { Log(Level::Info, info, flush, newLine, false); }
		static void Info(std::string_view info, bool flush = false, bool newLine = true) noexcept { Log(Level::Info, info, flush, newLine); }
		static void Warning(std::string_view warning, bool flush = false, bool newLine = true) noexcept { Log(Level::Warning, warning, flush, newLine); }
		static void Error(std::string_view error, bool flush = false, bool newLine = true) noexcept { Log(Level::Error, error, flush, newLine); }
		static void Critical(std::string_view error, bool newLine = true) noexcept { Log(Level::Critical, error, true, newLine); }

		static void CodeInfo(const Status& code, std::string_view msg, U32 line, const char* file) noexcept { LogStatusCode(Level::Info, code, msg, line, file); }
		static void CodeWarning(const Status& code, std::string_view msg, U32 line, const char* file) noexcept { LogStatusCode(Level::Warning, code, msg, line, file); }
		static void CodeError(const Status& code, std::string_view msg, U32 line, const char* file) noexcept { LogStatusCode(Level::Error, code, msg, line, file); }
		static void CodeCritical(const Status& code, std::string_view msg, U32 line, const char* file) noexcept { LogStatusCode(Level::Critical, code, msg, line, file); }

		static void Unformatted(bool error, std::string_view msg, bool newLine = true, bool indent = true) noexcept;
		static bool CreateLogDir(bool noLock = false) noexcept;
	};
}
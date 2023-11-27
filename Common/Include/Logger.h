#pragma once
#include "BasicTypes.h"
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
		enum class Level : U8 { Info, Warning, Error };

		static constexpr const char* LOG_FILE = "./Logs/log.txt";

		static inline bool firstUse = true;
		static inline std::shared_mutex consoleMutex;
		static inline std::shared_mutex fileMutex;

		static void Log(Level type, const std::string& log, bool flush, bool newLine = true, bool logToFile = true) noexcept;

	public:
		Logger() = delete;

		static bool CreateLogDir(bool noLock = false) noexcept;
		static void InfoNoFile(const std::string& info, bool flush = false, bool newLine = true) noexcept { Log(Level::Info, info, flush, newLine, false); }
		static void Info(const std::string& info, bool flush = false, bool newLine = true) noexcept { Log(Level::Info, info, flush, newLine); }
		static void Warning(const std::string& warning, bool flush = false, bool newLine = true) noexcept { Log(Level::Warning, warning, flush, newLine); }
		static void Error(const std::string& error, bool flush = false, bool newLine = true) noexcept { Log(Level::Error, error, flush, newLine); }
	};
}
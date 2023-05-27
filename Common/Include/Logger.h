#pragma once
#include "Types.h"
#include <string>

namespace ZE
{
	// Static logger service
	class Logger final
	{
		enum class Level : U8 { Info, Warning, Error };

		static inline bool firstUse = true;

		static void Log(Level type, const std::string& log, bool flush, bool logToFile = true);

	public:
		Logger() = delete;

		static void InfoNoFile(const std::string& info, bool flush = false) { Log(Level::Info, info, flush, false); }
		static void Info(const std::string& info, bool flush = false) { Log(Level::Info, info, flush); }
		static void Warning(const std::string& warning, bool flush = false) { Log(Level::Warning, warning, flush); }
		static void Error(const std::string& error, bool flush = false) { Log(Level::Error, error, flush); }
	};
}
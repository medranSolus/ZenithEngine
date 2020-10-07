#pragma once
#include <string>

class Logger
{
	enum class Level { Info, Warning, Error };

	static bool firstUse;

	static void Log(Level type, const std::string& log, bool noFile = false);

public:
	Logger() = delete;

	static inline void InfoNoFile(const std::string& info) { Log(Level::Info, info, true); }
	static inline void Info(const std::string& info) { Log(Level::Info, info); }
	static inline void Warning(const std::string& warning) { Log(Level::Warning, warning); }
	static inline void Error(const std::string& error) { Log(Level::Error, error); }
};
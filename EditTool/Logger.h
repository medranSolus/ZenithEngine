#pragma once
#include <string>

class Logger
{
	enum class Level { Info, Warning, Error };

	static bool firstUse;

	static void Log(Level type, const std::string& log);

public:
	Logger() = delete;

	static inline void Info(const std::string& info) { Log(Level::Info, info); }
	static inline void Warning(const std::string& warning) { Log(Level::Warning, warning); }
	static inline void Error(const std::string& error) { Log(Level::Error, error); }
};
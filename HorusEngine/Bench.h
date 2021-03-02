#pragma once
#include <string>
#include <cstdint>
#include <map>

class Bench
{
	std::string logFile;
	std::map<std::string, std::pair<uint64_t, uint64_t>> data;
	std::string lastTag = "";
	uint64_t stamp = 0;

	void Save();

public:
	inline Bench(const std::string& logFile = "log.txt") noexcept : logFile(logFile) {}
	~Bench();

	void Start(const std::string& sectionTag) noexcept;
	void Stop();
};

#define SET_BENCH() static Bench __bench
#define START_BENCH(sectionTag) __bench.Start(sectionTag)
#define STOP_BENCH() __bench.Stop()
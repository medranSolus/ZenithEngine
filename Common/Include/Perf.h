#pragma once
#include "Types.h"
#include <string>
#include <map>

class Perf
{
	static constexpr const char* LOG_FILE = "perf_log.txt";

	std::map<std::string, std::pair<U64, U64>> data;
	std::string lastTag = "";
	U64 stamp = 0;

	void Save();

public:
	Perf() = default;
	Perf(Perf&&) = default;
	Perf(const Perf&) = default;
	Perf& operator=(Perf&&) = default;
	Perf& operator=(const Perf&) = default;
	~Perf();

	void Start(const std::string& sectionTag) noexcept;
	void Stop() noexcept;
};

#define PERF_SET() static Perf __perf
#define PERF_START(sectionTag) __perf.Start(sectionTag)
#define PERF_STOP() __perf.Stop()
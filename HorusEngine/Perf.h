#pragma once
#include <string>
#include <cstdint>
#include <map>

class Perf
{
	static constexpr const char* LOG_FILE = "perf_log.txt";

	std::map<std::string, std::pair<uint64_t, uint64_t>> data;
	std::string lastTag = "";
	uint64_t stamp = 0;

	void Save();

public:
	Perf() = default;
	~Perf();

	void Start(const std::string& sectionTag) noexcept;
	void Stop();
};

#define PERF_SET() static Perf __perf
#define PERF_START(sectionTag) __perf.Start(sectionTag)
#define PERF_STOP() __perf.Stop()
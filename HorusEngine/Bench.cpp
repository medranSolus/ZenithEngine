#include "Bench.h"
#include <fstream>
#include <intrin.h>
#pragma intrinsic(__rdtsc, __faststorefence)

void Bench::Save()
{
	std::ofstream fout(logFile, std::ios_base::app);
	if (!fout.good())
		return;
	for (auto& x : data)
		fout << '[' << x.first << "] Avg cycles: " << x.second.first << ", tests: " << x.second.second << std::endl;
	fout.close();
}

Bench::~Bench()
{
	if (data.size() > 0)
	{
		for (auto& x : data)
			if (x.second.second != 0)
				x.second.first /= x.second.second;
		Save();
	}
}

void Bench::Start(const std::string& sectionTag) noexcept
{
	data.emplace(sectionTag, std::make_pair(0ULL, 0ULL));
	lastTag = sectionTag;
#ifndef WIN32
	__faststorefence();
#else
	_asm volatile ("lfence");
#endif
	stamp = __rdtsc();
}

void Bench::Stop()
{
	const uint64_t end = __rdtsc();
#ifndef WIN32
	__faststorefence();
#else
	_asm volatile ("lfence");
#endif
	data.at(lastTag).first += end - stamp;
	++data.at(lastTag).second;
	stamp = 0;
	lastTag = "";
}
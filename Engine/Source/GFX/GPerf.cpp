#include "GFX/GPerf.h"
#include <fstream>

namespace ZE::GFX
{
	void GPerf::Save()
	{
		std::ofstream fout(LOG_FILE, std::ios_base::app);
		if (!fout.good())
			return;
		for (auto& x : data)
		{
			const char* apiString = nullptr;
			ZE_API_BACKEND_CALL_RET(backend, apiString, GetApiString);
			fout << '[' << x.first << "] <" << apiString << "> Avg micro seconds : "
				<< x.second.first << ", tests: " << x.second.second << std::endl;
		}
		data.clear();
		fout.close();
	}

	void GPerf::Start(const std::string& sectionTag) noexcept
	{
		if (data.find(sectionTag) == data.end())
			data.emplace(sectionTag, std::make_pair(0.0L, 0ULL));
		lastTag = sectionTag;
		ZE_API_BACKEND_CALL(backend, Start);
	}

	void GPerf::Stop() noexcept
	{
		long double time;
		ZE_API_BACKEND_CALL_RET(backend, time, Stop);
		if (time != 0.0L)
			data.at(lastTag).first += (time - data.at(lastTag).first) / ++data.at(lastTag).second;
		lastTag = "";
	}
}
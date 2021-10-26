#include "GFX/GPerf.h"

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

	void GPerf::Start(CommandList& cl, const std::string& sectionTag) noexcept
	{
		if (data.find(sectionTag) == data.end())
			data.emplace(sectionTag, std::make_pair(0.0L, 0ULL));
		lastTags.push_back(sectionTag);
		ZE_API_BACKEND_CALL(backend, Start, cl);
	}

	void GPerf::Collect(Device& dev) noexcept
	{
		for (auto it = lastTags.rbegin(); it != lastTags.rend(); ++it)
		{
			long double time = 0.0L;
			ZE_API_BACKEND_CALL_RET(backend, time, GetData, dev);
			if (time != 0.0L)
			{
				auto& dataPoint = data.at(*it);
				dataPoint.first += (time - dataPoint.first) / ++dataPoint.second;
			}
		}
		lastTags.clear();
	}
}
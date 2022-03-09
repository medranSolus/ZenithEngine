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
			ZE_API_BACKEND_CALL_RET(apiString, GetApiString);
			fout << '[' << x.first << "] <" << apiString << "> Avg micro seconds: "
				<< x.second.first << ", tests: " << x.second.second << std::endl;
		}
		data.clear();
		fout.close();
	}

	void GPerf::Start(CommandList& cl, const std::string& sectionTag) noexcept
	{
		if (data.find(sectionTag) == data.end())
			data.emplace(sectionTag, std::make_pair(0.0L, 0ULL));
		lastTag = sectionTag;
		ZE_API_BACKEND_CALL(Start, cl);
	}

	void GPerf::Collect(Device& dev) noexcept
	{
		long double time = 0.0L;
		ZE_API_BACKEND_CALL_RET(time, GetData, dev);
		if (time != 0.0L)
		{
			auto& dataPoint = data.at(lastTag);
			dataPoint.first += (time - dataPoint.first) / static_cast<long double>(++dataPoint.second);
		}
		lastTag = "";
	}
}
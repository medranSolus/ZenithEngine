#include "GFX/GPerf.h"
#include "Settings.h"
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
			fout << '[' << x.first << "] " << GetApiString() << " Avg micro seconds : "
				<< x.second.first << ", tests: " << x.second.second << std::endl;
		}
		data.clear();
		fout.close();
	}

	GPerf::~GPerf()
	{
		if (data.size() > 0)
			Save();
	}

	void GPerf::Init(Device& dev)
	{
		if (instance)
			delete instance;
		instance = Settings::GetGfxApi().MakeGpuPerf(dev);
	}

	void GPerf::Start(const std::string& sectionTag) noexcept
	{
		if (data.find(sectionTag) == data.end())
			data.emplace(sectionTag, std::make_pair(0.0L, 0ULL));
		lastTag = sectionTag;
		StartImpl();
	}

	void GPerf::Stop() noexcept
	{
		StopImpl();
		lastTag = "";
	}
}
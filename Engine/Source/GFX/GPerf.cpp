#include "GFX/GPerf.h"

namespace ZE::GFX
{
	void GPerf::Save() noexcept
	{
		if (IsInitialized())
		{
			std::ofstream fout(LOG_FILE, std::ios_base::app);
			if (!fout.good())
				return;
			for (auto& x : data)
			{
				const char* apiString = nullptr;
				ZE_RHI_BACKEND_CALL_RET_VAR(apiString, GetApiString);
				fout << '[' << x.first << "] <" << apiString << "> Avg micro seconds: "
					<< x.second.first << ", tests: " << x.second.second << std::endl;
			}
			data.clear();
			fout.close();
		}
	}

	GPerf& GPerf::Get(Device& dev) noexcept
	{
		static GPerf instance;
		if (!instance.IsCreated())
		{
			instance.singletonStatus[1] = true;
			Expected<GPerf> createResult = Create(dev);
			if (createResult)
			{
				instance = std::move(createResult.value());
				instance.singletonStatus[0] = true;
			}
			else
			{
				ZE_CODE_ERROR(createResult.error(), "Failed to create GPerf instance, running without GPU data profiling!");
			}
		}
		return instance;
	}

	void GPerf::Start(CommandList& cl, const std::string& sectionTag) noexcept
	{
		if (IsInitialized())
		{
			if (data.find(sectionTag) == data.end())
				data.emplace(sectionTag, std::make_pair(0.0L, 0ULL));
			lastTag = sectionTag;
			ZE_RHI_BACKEND_CALL(Start, cl);
		}
	}

	void GPerf::Collect(Device& dev) noexcept
	{
		if (IsInitialized())
		{
			long double time = 0.0L;
			ZE_RHI_BACKEND_CALL_RET_VAR(time, GetData, dev);
			if (time != 0.0L)
			{
				auto& dataPoint = data.at(lastTag);
				dataPoint.first += (time - dataPoint.first) / Utils::SafeCast<long double>(++dataPoint.second);
			}
			lastTag = "";
		}
	}
}
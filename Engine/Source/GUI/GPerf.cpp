#include "GFX/GPerf.h"
#include "Exception/GfxExceptionMacros.h"
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
			fout << '[' << x.first << "] Avg micro seconds: " << x.second.first << ", tests: " << x.second.second << std::endl;
		}
		data.clear();
		fout.close();
	}

	GPerf::GPerf(Graphics& gfx)
	{
		ZE_GFX_ENABLE_ALL(gfx);
		D3D11_QUERY_DESC desc;
		desc.MiscFlags = 0;
		desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		ZE_GFX_THROW_FAILED(gfx.device->CreateQuery(&desc, &disjoint));
		desc.Query = D3D11_QUERY_TIMESTAMP;
		ZE_GFX_THROW_FAILED(gfx.device->CreateQuery(&desc, &begin));
		ZE_GFX_THROW_FAILED(gfx.device->CreateQuery(&desc, &end));
		gfx.device->GetImmediateContext(&ctx);
	}

	GPerf::~GPerf()
	{
		if (data.size() > 0)
			Save();
	}

	void GPerf::Start(const std::string& sectionTag) noexcept
	{
		if (data.find(sectionTag) == data.end())
			data.emplace(sectionTag, std::make_pair(0.0L, 0ULL));
		lastTag = sectionTag;
		ctx->Begin(disjoint.Get());
		ctx->End(begin.Get());
	}

	void GPerf::Stop() noexcept
	{
		ctx->End(end.Get());
		ctx->End(disjoint.Get());
		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT dataDisjoint;
		while (ctx->GetData(disjoint.Get(), &dataDisjoint, sizeof(D3D11_QUERY_DATA_TIMESTAMP_DISJOINT), 0) != S_OK);
		if (!dataDisjoint.Disjoint)
		{
			U64 ticksBegin, ticksEnd;
			while (ctx->GetData(begin.Get(), &ticksBegin, sizeof(U64), 0) != S_OK);
			while (ctx->GetData(end.Get(), &ticksEnd, sizeof(U64), 0) != S_OK);
			long double megaFrequency = static_cast<long double>(dataDisjoint.Frequency) / 1000000.0L;
			data.at(lastTag).first += ((ticksEnd - ticksBegin) / megaFrequency - data.at(lastTag).first) / ++data.at(lastTag).second;
		}
		lastTag = "";
	}
}
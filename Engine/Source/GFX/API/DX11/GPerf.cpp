#include "GFX/API/DX11/GPerf.h"

namespace ZE::GFX::API::DX11
{
	GPerf::GPerf(GFX::Device& dev)
	{
		ZE_DX_ENABLE(dev.Get().dx11);

		D3D11_QUERY_DESC1 desc = {};
		desc.MiscFlags = 0;
		desc.ContextType = D3D11_CONTEXT_TYPE_ALL;

		desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		ZE_DX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateQuery1(&desc, &disjoint));

		desc.Query = D3D11_QUERY_TIMESTAMP;
		ZE_DX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateQuery1(&desc, &begin));
		ZE_DX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateQuery1(&desc, &end));
	}

	void GPerf::Start(GFX::CommandList& cl) noexcept
	{
		cl.Get().dx11.GetContext()->Begin(disjoint.Get());
		cl.Get().dx11.GetContext()->End(begin.Get());
	}

	void GPerf::Stop(GFX::CommandList& cl) const noexcept
	{
		cl.Get().dx11.GetContext()->End(end.Get());
		cl.Get().dx11.GetContext()->End(disjoint.Get());
	}

	long double GPerf::GetData(GFX::Device& dev) noexcept
	{
		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT dataDisjoint;
		while (dev.Get().dx11.GetMainContext()->GetData(disjoint.Get(), &dataDisjoint, sizeof(D3D11_QUERY_DATA_TIMESTAMP_DISJOINT), 0) != S_OK);

		if (!dataDisjoint.Disjoint)
		{
			U64 ticksBegin, ticksEnd;
			while (dev.Get().dx11.GetMainContext()->GetData(begin.Get(), &ticksBegin, sizeof(U64), 0) != S_OK);
			while (dev.Get().dx11.GetMainContext()->GetData(end.Get(), &ticksEnd, sizeof(U64), 0) != S_OK);

			const long double megaFrequency = Utils::SafeCast<long double>(dataDisjoint.Frequency) / 1000000.0L;
			return Utils::SafeCast<long double>(ticksEnd - ticksBegin) / megaFrequency;
		}
		return 0.0L;
	}
}
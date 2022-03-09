#include "GFX/API/DX11/GPerf.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11
{
	GPerf::GPerf(GFX::Device& dev)
	{
		ZE_GFX_ENABLE(dev.Get().dx11);

		D3D11_QUERY_DESC desc;
		desc.MiscFlags = 0;
		desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateQuery(&desc, &disjoint));
		desc.Query = D3D11_QUERY_TIMESTAMP;
		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateQuery(&desc, &begin));
		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateQuery(&desc, &end));
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

			const long double megaFrequency = static_cast<long double>(dataDisjoint.Frequency) / 1000000.0L;
			return static_cast<long double>(ticksEnd - ticksBegin) / megaFrequency;
		}
		return 0.0L;
	}
}
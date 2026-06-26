#pragma once
#include "GFX/CommandList.h"

namespace ZE::RHI::DX11
{
	class GPerf final
	{
		DX::ComPtr<IQuery> disjoint;
		DX::ComPtr<IQuery> begin;
		DX::ComPtr<IQuery> end;

	public:
		GPerf() = default;
		ZE_CLASS_MOVE(GPerf);
		~GPerf() = default;

		static constexpr const char* GetApiString() noexcept { return "DX11"; }

		static Expected<GPerf> Create(GFX::Device& dev) noexcept;

		void Start(GFX::CommandList& cl) noexcept;
		void Stop(GFX::CommandList& cl) const noexcept;
		long double GetData(GFX::Device& dev) noexcept;
	};
}
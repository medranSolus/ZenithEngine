#pragma once
#include "GFX/CommandList.h"

namespace ZE::RHI::VK
{
	class GPerf final
	{
	public:
		GPerf() = default;
		GPerf(GFX::Device& dev);
		ZE_CLASS_MOVE(GPerf);
		~GPerf() = default;

		static constexpr const char* GetApiString() noexcept { return "VK"; }

		void Start(GFX::CommandList& cl) noexcept;
		void Stop(GFX::CommandList& cl) const noexcept;
		long double GetData(GFX::Device& dev) noexcept;
	};
}
#include "GFX/Data/CBuffer/DCBLayoutCodex.h"

namespace ZE::GFX::Data::CBuffer
{
	DCBLayoutFinal DCBLayoutCodex::Resolve(DCBLayout&& layout) noexcept
	{
		auto key = layout.GetSignature();
		auto& map = Get().map;
		const auto it = map.find(key);
		if (it != map.end())
			return { it->second };

		auto result = map.insert({ std::move(key), layout.Finalize() });
		return { result.first->second };
	}
}
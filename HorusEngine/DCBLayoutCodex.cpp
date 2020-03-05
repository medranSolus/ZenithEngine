#include "DCBLayoutCodex.h"

namespace GFX::Data::CBuffer
{
	DCBLayoutFinal DCBLayoutCodex::Resolve(DCBLayout&& layout) noexcept(!IS_DEBUG)
	{
		auto key = layout.GetSignature();
		auto& map = Get().map;
		const auto it = map.find(key);
		if (it != map.end())
		{
			layout.Clear();
			return { it->second };
		}
		auto result = map.insert({ std::move(key), layout.Finalize() });
		return { result.first->second };
	}
}
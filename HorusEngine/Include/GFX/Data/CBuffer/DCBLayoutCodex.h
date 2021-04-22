#pragma once
#include "DCBLayout.h"
#include "DCBLayoutFinal.h"
#include <unordered_map>

namespace GFX::Data::CBuffer
{
	class DCBLayoutCodex final
	{
		std::unordered_map<std::string, std::shared_ptr<DCBLayoutElement>> map;

		static DCBLayoutCodex& Get() noexcept { static DCBLayoutCodex codex; return codex; }

		DCBLayoutCodex() = default;

	public:
		~DCBLayoutCodex() = default;

		static DCBLayoutFinal Resolve(DCBLayout&& layout) noexcept;
	};
}
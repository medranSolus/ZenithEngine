#pragma once
#include "DCBLayoutBase.h"

namespace GFX::Data::CBuffer
{
	class DCBLayoutFinal : public DCBLayoutBase
	{
		friend class DCBLayoutCodex;

		inline DCBLayoutFinal(std::shared_ptr<DCBLayoutElement> root) noexcept : DCBLayoutBase(std::move(root)) {}

	public:
		virtual ~DCBLayoutFinal() = default;

		inline std::shared_ptr<DCBLayoutElement> GetRoot() const noexcept { return root; }
		inline std::shared_ptr<DCBLayoutElement> FlushRoot() noexcept { return std::move(root); }

		inline const DCBLayoutElement& operator[](const std::string& key) const noexcept(!IS_DEBUG) { return (*root)[key]; }
	};
}

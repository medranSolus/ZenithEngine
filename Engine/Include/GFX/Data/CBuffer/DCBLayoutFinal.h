#pragma once
#include "DCBLayoutBase.h"

namespace ZE::GFX::Data::CBuffer
{
	class DCBLayoutFinal final : public DCBLayoutBase
	{
		friend class DCBLayoutCodex;

		DCBLayoutFinal(std::shared_ptr<DCBLayoutElement> root) noexcept : DCBLayoutBase(std::move(root)) {}
		DCBLayoutFinal(DCBLayoutFinal&&) = default;
		DCBLayoutFinal(const DCBLayoutFinal&) = default;
		DCBLayoutFinal& operator=(DCBLayoutFinal&&) = default;
		DCBLayoutFinal& operator=(const DCBLayoutFinal&) = default;

	public:
		virtual ~DCBLayoutFinal() = default;

		std::shared_ptr<DCBLayoutElement> GetRoot() const noexcept { return root; }
		std::shared_ptr<DCBLayoutElement> FlushRoot() noexcept { return std::move(root); }

		const DCBLayoutElement& operator[](const std::string& key) const noexcept { return (*root)[key]; }
	};
}
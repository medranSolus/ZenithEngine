#pragma once
#include "DCBLayoutBase.h"

namespace GFX::Data::CBuffer
{
	class DCBLayout : public DCBLayoutBase
	{
		friend class DCBLayoutCodex;

		inline void Clear() noexcept { *this = DCBLayout{}; }

		std::shared_ptr<DCBLayoutElement> Finalize() noexcept;

	public:
		inline DCBLayout() noexcept(!IS_DEBUG)
			: DCBLayoutBase(std::shared_ptr<DCBLayoutElement>(new DCBLayoutElement(ElementType::Struct))) {}
		DCBLayout(const DCBLayout&) = default;
		DCBLayout& operator=(const DCBLayout&) = default;
		virtual ~DCBLayout() = default;

		inline DCBLayoutElement& Add(ElementType typeAdded, std::string name) noexcept(!IS_DEBUG) { return root->Add(typeAdded, name); }

		inline DCBLayoutElement& operator[](const std::string& key) noexcept(!IS_DEBUG) { return (*root)[key]; }
	};
}
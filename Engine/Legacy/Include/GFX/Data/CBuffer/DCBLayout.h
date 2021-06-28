#pragma once
#include "DCBLayoutBase.h"

namespace ZE::GFX::Data::CBuffer
{
	class DCBLayout : public DCBLayoutBase
	{
		friend class DCBLayoutCodex;

		std::shared_ptr<DCBLayoutElement> Finalize() noexcept { root->Finalize(0); return std::move(root); }

	public:
		DCBLayout() noexcept : DCBLayoutBase(std::make_shared<DCBLayoutElement>(DCBLayoutElement{ ElementType::Struct })) {}
		DCBLayout(DCBLayout&&) = default;
		DCBLayout(const DCBLayout&) = default;
		DCBLayout& operator=(DCBLayout&&) = default;
		DCBLayout& operator=(const DCBLayout&) = default;
		virtual ~DCBLayout() = default;

		DCBLayoutElement& Add(ElementType typeAdded, std::string name) noexcept { return root->Add(typeAdded, name); }
		DCBLayoutElement& operator[](const std::string& key) noexcept { return (*root)[key]; }
	};
}
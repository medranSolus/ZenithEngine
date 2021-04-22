#pragma once
#include "DCBLayoutElement.h"

namespace GFX::Data::CBuffer
{
	class DCBLayoutBase
	{
	protected:
		std::shared_ptr<DCBLayoutElement> root = nullptr;

		DCBLayoutBase(std::shared_ptr<DCBLayoutElement>&& root) noexcept : root(std::move(root)) {}
		DCBLayoutBase(DCBLayoutBase&&) = default;
		DCBLayoutBase(const DCBLayoutBase&) = default;
		DCBLayoutBase& operator=(DCBLayoutBase&&) = default;
		DCBLayoutBase& operator=(const DCBLayoutBase&) = default;

	public:
		virtual ~DCBLayoutBase() = default;

		U64 GetByteSize() const { return root->GetByteSize(); }
		std::string GetSignature() const noexcept { return root->GetSignature(); }
	};
}
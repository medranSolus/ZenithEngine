#pragma once
#include "DCBLayoutElement.h"

namespace GFX::Data::CBuffer
{
	class DCBLayoutBase
	{
	protected:
		std::shared_ptr<DCBLayoutElement> root = nullptr;

		inline DCBLayoutBase(std::shared_ptr<DCBLayoutElement> root) noexcept : root(std::move(root)) {}
		DCBLayoutBase(const DCBLayoutBase&) = default;
		DCBLayoutBase& operator=(const DCBLayoutBase&) = default;

	public:
		virtual ~DCBLayoutBase() = default;

		inline size_t GetByteSize() const { return root->GetByteSize(); }
		inline std::string GetSignature() const noexcept(!IS_DEBUG) { return root->GetSignature(); }
	};
}
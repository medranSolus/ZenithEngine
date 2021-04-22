#pragma once
#include "DCBLayoutCodex.h"
#include "DCBElement.h"

namespace GFX::Data::CBuffer
{
	class DynamicCBuffer final
	{
		std::vector<U8> bytes;
		std::shared_ptr<DCBLayoutElement> root = nullptr;

	public:
		DynamicCBuffer(DCBLayout&& layout) noexcept
			: DynamicCBuffer(DCBLayoutCodex::Resolve(std::forward<DCBLayout>(layout))) {}
		DynamicCBuffer(DCBLayoutFinal&& layout)
			: bytes(layout.GetByteSize()), root(layout.FlushRoot()) {}
		DynamicCBuffer(const DCBLayoutFinal& layout) noexcept
			: bytes(layout.GetByteSize()), root(layout.GetRoot()) {}
		DynamicCBuffer(DynamicCBuffer&&) = default;
		DynamicCBuffer(const DynamicCBuffer&) = default;
		DynamicCBuffer& operator=(DynamicCBuffer&&) = default;
		DynamicCBuffer& operator=(const DynamicCBuffer&) = default;
		~DynamicCBuffer() = default;

		U64 GetByteSize() const noexcept { return bytes.size(); }
		const U8* GetData() const noexcept { return bytes.data(); }
		const DCBLayoutElement& GetRootElement() const noexcept { return *root; }
		std::shared_ptr<DCBLayoutElement> GetRoot() const noexcept { return root; }

		DCBElementConst operator[](const std::string& key) const noexcept { return const_cast<DynamicCBuffer&>(*this)[key]; }
		DCBElement operator[](const std::string& key) noexcept { return { &(*root)[key], bytes.data(), 0 }; }
	};
}
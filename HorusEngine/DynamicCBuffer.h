#pragma once
#include "DCBLayoutCodex.h"
#include "DCBElement.h"

namespace GFX::Data::CBuffer
{
	class DynamicCBuffer
	{
		std::vector<char> bytes;
		std::shared_ptr<DCBLayoutElement> root = nullptr;

	public:
		inline DynamicCBuffer(DCBLayout&& layout) noexcept(!IS_DEBUG)
			: DynamicCBuffer(DCBLayoutCodex::Resolve(std::move(layout))) {}
		inline DynamicCBuffer(const DCBLayoutFinal& layout) noexcept(!IS_DEBUG)
			: bytes(root->GetEndOffset()), root(layout.GetRoot()) {}
		inline DynamicCBuffer(DCBLayoutFinal&& layout)
			: bytes(layout.GetByteSize()), root(std::move(layout.GetRoot())) {}
		inline DynamicCBuffer(const DynamicCBuffer& buffer) noexcept
			: bytes(buffer.bytes), root(buffer.root) {}
		inline DynamicCBuffer(DynamicCBuffer&& buffer) noexcept
			: bytes(std::move(buffer.bytes)), root(std::move(buffer.root)) {}
		~DynamicCBuffer() = default;

		inline size_t GetByteSize() const noexcept { return bytes.size(); }
		inline const char* GetData() const noexcept { return bytes.data(); }
		inline const DCBLayoutElement& GetRootElement() const noexcept { return *root; }
		inline std::shared_ptr<DCBLayoutElement> GetRoot() const noexcept { return root; }

		void Copy(const DynamicCBuffer& buffer) noexcept(!IS_DEBUG);

		inline DCBElementConst operator[](const std::string& key) const noexcept(!IS_DEBUG) { return const_cast<DynamicCBuffer&>(*this)[key]; }

		DCBElement operator[](const std::string& key) noexcept(!IS_DEBUG) { return { &(*root)[key], bytes.data(), 0U }; }
	};
}
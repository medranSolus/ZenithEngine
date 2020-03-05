#include "DynamicCBuffer.h"

namespace GFX::Data::CBuffer
{
	inline DynamicCBuffer::DynamicCBuffer(DCBLayout&& layout) noexcept(!IS_DEBUG)
		: DynamicCBuffer(DCBLayoutCodex::Resolve(std::move(layout))) {}

	inline DynamicCBuffer::DynamicCBuffer(const DCBLayoutFinal& layout) noexcept(!IS_DEBUG)
		: root(layout.GetRoot()), bytes(root->GetEndOffset()) {}

	inline DynamicCBuffer::DynamicCBuffer(DCBLayoutFinal&& layout) noexcept(!IS_DEBUG)
		: root(layout.FlushRoot()), bytes(root->GetEndOffset()) {}

	constexpr DynamicCBuffer::DynamicCBuffer(const DynamicCBuffer& buffer) noexcept
		: bytes(buffer.bytes), root(buffer.root) {}

	constexpr DynamicCBuffer::DynamicCBuffer(DynamicCBuffer&& buffer) noexcept
		: bytes(std::move(buffer.bytes)), root(std::move(buffer.root)) {}

	void DynamicCBuffer::Copy(const DynamicCBuffer& buffer) noexcept(!IS_DEBUG)
	{
		assert(&GetRootElement() == &buffer.GetRootElement());
		std::copy(buffer.bytes.begin(), buffer.bytes.end(), bytes.begin());
	}
}
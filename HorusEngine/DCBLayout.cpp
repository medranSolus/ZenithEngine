#include "DCBLayout.h"

namespace GFX::Data::CBuffer
{
	std::shared_ptr<DCBLayoutElement> DCBLayout::Finalize() noexcept
	{
		auto tree = std::move(root);
		tree->Finalize(0U);
		Clear();
		return std::move(tree);
	}
}
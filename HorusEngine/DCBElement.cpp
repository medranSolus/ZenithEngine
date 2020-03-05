#include "DCBElement.h"

namespace GFX::Data::CBuffer
{
	DCBElement DCBElement::operator[](size_t index) const noexcept(!IS_DEBUG)
	{
		const auto indexData = layout->CalculateIndexOffset(offset, index);
		return { indexData.second, bytes, indexData.first };
	}
}
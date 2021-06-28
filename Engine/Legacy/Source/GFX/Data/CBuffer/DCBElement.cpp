#include "GFX/Data/CBuffer/DCBElement.h"

namespace ZE::GFX::Data::CBuffer
{
	DCBElement DCBElement::operator[](U64 index) const noexcept
	{
		const auto indexData = layout->CalculateIndexOffset(offset, index);
		return { indexData.second, bytes, indexData.first };
	}
}
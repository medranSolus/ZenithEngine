#include "GFX/Data/CBuffer/DCBElementConst.h"

namespace GFX::Data::CBuffer
{
	DCBElementConst DCBElementConst::operator[](U64 index) const noexcept
	{
		const auto indexData = layout->CalculateIndexOffset(offset, index);
		return { indexData.second, bytes, indexData.first };
	}
}
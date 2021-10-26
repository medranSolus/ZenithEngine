#pragma once
#include "Types.h"

namespace ZE::GFX::Resource
{
	// Primitive topology described by index data
	enum class TopologyType : U8
	{
		Undefined,
		Point,
		Line,
		Triangle,
		ControlPoint
	};

	// Possible logical index data ordering in memory
	enum class TopologyOrder : U8
	{
		Undefined,
		List,
		Strip,
		ListAdjacency,
		StripAdjacency,
		PatchList1,
		PatchList2,
		PatchList3,
		PatchList4,
		PatchList5,
		PatchList6,
		PatchList7,
		PatchList8,
		PatchList9,
		PatchList10,
		PatchList11,
		PatchList12,
		PatchList13,
		PatchList14,
		PatchList15,
		PatchList16,
		PatchList17,
		PatchList18,
		PatchList19,
		PatchList20,
		PatchList21,
		PatchList22,
		PatchList23,
		PatchList24,
		PatchList25,
		PatchList26,
		PatchList27,
		PatchList28,
		PatchList29,
		PatchList30,
		PatchList31,
		PatchList32
	};
}
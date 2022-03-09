#pragma once

namespace ZE::Data
{
	// Unique idenitfier of single Entity
	typedef U64 EID;

	// Component representing single object in the scene
	struct Entity
	{
		static constexpr EID INVALID_ID = UINT64_MAX;

		EID ID;
		EID ParentID = INVALID_ID;
	};
}
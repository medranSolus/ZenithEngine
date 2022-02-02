#pragma once

namespace ZE::Data
{
	// Unique idenitfier of single Entity
	typedef U64 EID;

	// Component representing single object in the scene
	struct Entity
	{
		static constexpr EID INVALID_ID = -1;

		EID ID;
		EID ParentID = INVALID_ID;
	};
}
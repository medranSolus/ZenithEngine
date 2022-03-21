#pragma once
#include "WarningGuardOn.h"
#include "entt/entt.hpp"
#include "WarningGuardOff.h"

namespace ZE
{
	namespace Data
	{
		// Main component data storage object
		typedef entt::registry Storage;
	}

	// Identifier of single entity
	typedef entt::entity EID;

	// Identifier of parent for given entity
	typedef EID ParentID;

	// Identifier of invalid entity
	inline constexpr const EID& INVALID_EID = entt::null;
}

// Check if given entity id is valid
#define ZE_VALID_EID(eid) ZE_ASSERT(eid != INVALID_EID, "Invalid entity!")
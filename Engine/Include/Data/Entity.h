#pragma once
ZE_WARNING_PUSH
#include "entt/entt.hpp"
ZE_WARNING_POP

namespace ZE
{
	// Identifier of single entity
	typedef entt::entity EID;

	// Identifier of parent for given entity
	struct ParentID { EID ID; };

	// List of children for given entity
	struct Children
	{
		std::vector<EID> Childs;
	};

	// Identifier of invalid entity
	inline constexpr const EID& INVALID_EID = entt::null;

	namespace Data
	{
		// Identifier of single geometry data
		struct MeshID { EID ID; };

		// Main component data storage object
		typedef entt::registry Storage;
	}
}

// Check if given entity id is valid
#define ZE_VALID_EID(eid) ZE_ASSERT(eid != INVALID_EID, "Invalid entity!")
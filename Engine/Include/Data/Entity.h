#pragma once
ZE_WARNING_PUSH
#include "entt/entt.hpp"
ZE_WARNING_POP

namespace ZE
{
	// Identifier of single entity
	typedef entt::entity EID;

	// Identifier of invalid entity
	inline constexpr const EID& INVALID_EID = entt::null;

	namespace Data
	{
		// Main component data storage object
		typedef entt::registry Storage;

		// Identifier of parent for given entity
		struct ParentID { EID ID = INVALID_EID; };

		// List of children for given entity
		struct ChildrenIDs
		{
			std::vector<EID> Childs;
		};

		// Identifier of single geometry data, objects poiting to it might be part of the assets storage pool
		struct MeshID { EID ID = INVALID_EID; };
		
		// Identifier of single material data, objects poiting to it might be part of the assets storage pool
		struct MaterialID { EID ID = INVALID_EID; };

		// If data pool is used concurrently by multiple threads, it have to be assured that all pools are created before using them
		template<typename Type, typename ...Other>
		constexpr void AssureEntityPools(Storage& data) noexcept
		{
			if ((!data.storage(entt::type_hash<Type>()) || ... || !data.storage(entt::type_hash<Other>())))
				(data.storage<Type>(), ..., data.storage<Other>());
		}
	}
}

// Check if given entity id is valid
#define ZE_VALID_EID(eid) ZE_ASSERT(eid != INVALID_EID, "Invalid entity!")
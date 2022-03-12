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
}

#pragma once
#include "Settings.h"

namespace ZE::Data
{
	// Component allowing to place physical object in the scene.
	// Ordering by non-decreasing parent, root entities at the end.
	// If child entity have Transform then all parents should have said component too
	struct Transform
	{
		Float4 Rotation;
		Float3 Position;
		Float3 Scale;
	};
	struct TransformGlobal : public Transform {};
	struct TransformPrevious : public TransformGlobal {};

	// Assure that all transform components are registered as pools in data storage
	constexpr void InitTransformComponents() noexcept { Settings::AssureEntityPools<Transform, TransformGlobal, TransformPrevious>(); }
}
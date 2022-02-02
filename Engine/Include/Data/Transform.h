#pragma once

namespace ZE::Data
{
	// Component allowing to place physical object in the scene
	struct Transform
	{
		Float4 Rotation;
		Float3 Position;
		Float3 Scale;
	};
}
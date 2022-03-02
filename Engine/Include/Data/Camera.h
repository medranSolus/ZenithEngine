#pragma once

namespace ZE::Data
{
	// Perspective projection data
	struct Projection
	{
		float FOV;
		float ScreenRatio;
		float NearClip;
		float FarClip;
	};

	// Component describing camera params
	struct Camera
	{
		Float3 EyeDirection;
		Float3 UpVector;
		Projection Projection;
	};
}
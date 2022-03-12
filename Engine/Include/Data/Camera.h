#pragma once

namespace ZE::Data
{
	// Perspective projection data
	struct Projection
	{
		float FOV;
		float ViewRatio;
		float NearClip;
		float FarClip;
	};

	// Component describing camera params
	struct Camera
	{
		static constexpr float ROTATE_EPSILON = 0.000001f - FLT_EPSILON;
		static constexpr float FLIP_EPSILON = 16.0f * FLT_EPSILON;

		Float3 EyeDirection;
		Float3 UpVector;
		Projection Projection;
	};

	// Types of camera supported by several utility methods
	enum class CameraType : bool
	{
		// Classic person-like camera capable of moving in single plane
		Person,
		// Camera that moves into direction that currently looks at
		Floating
	};
}
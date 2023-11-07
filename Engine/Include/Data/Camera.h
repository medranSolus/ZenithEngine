#pragma once
#include "Entity.h"

namespace ZE::Data
{
	// Perspective projection data
	struct Projection
	{
		float FOV;
		float ViewRatio;
		float NearClip;
		// Projection space jitter offsets
		float JitterX;
		float JitterY;
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

#pragma region Functions
	// Get frustum from infinite far plan reversed clip space projection
	Math::BoundingFrustum GetFrustum(Matrix proj, float maxDistance) noexcept;
	// Get symetrical vertical FOV projection matrix with infinite far plane for reversed clip space
	Matrix GetProjectionMatrix(const Projection& proj, UInt2 viewportSize = { 0, 0 }) noexcept;
	// Move any camera in Right-Left axis (dX < 0 - movement left, dX > 0 - movement right)
	void MoveCameraX(Storage& registry, EID camera, float dX, CameraType type) noexcept;
	// Move any camera in Up-Down axis (dY < 0 - movement down, dY > 0 - movement up)
	void MoveCameraY(Storage& registry, EID camera, float dY, CameraType type) noexcept;
	// Move any camera in Forward-Backward axis (dZ < 0 - movement backward, dZ > 0 - movement forward)
	void MoveCameraZ(Storage& registry, EID camera, float dZ, CameraType type) noexcept;
	// Roll any camera view in Left-Right direction (delta < 0 - roll counterclockwise, delta > 0 roll clockwise)
	void RollCamera(Storage& registry, EID camera, float delta, CameraType type) noexcept;
	// Rotate any camera view (angleDX < 0 - rotate down, angleDX > 0 - rotate up; angleDY < 0 rotate left, angleDY > 0 - rotate right)
	void RotateCamera(Storage& registry, EID camera, float angleDX, float angleDY, CameraType type) noexcept;

	// Move person camera in Right-Left axis (dX < 0 - movement left, dX > 0 - movement right)
	void MovePersonCameraX(Storage& registry, EID camera, float dX) noexcept;
	// Move person camera in Up-Down axis (dY < 0 - movement down, dY > 0 - movement up)
	void MovePersonCameraY(Storage& registry, EID camera, float dY) noexcept;
	// Move person camera in Forward-Backward axis (dZ < 0 - movement backward, dZ > 0 - movement forward)
	void MovePersonCameraZ(Storage& registry, EID camera, float dZ) noexcept;
	// Roll person camera view in Left-Right direction (delta < 0 - roll counterclockwise, delta > 0 roll clockwise)
	void RollPersonCamera(Storage& registry, EID camera, float delta) noexcept;
	// Rotate person camera view (angleDX < 0 - rotate down, angleDX > 0 - rotate up; angleDY < 0 rotate left, angleDY > 0 - rotate right)
	void RotatePersonCamera(Storage& registry, EID camera, float angleDX, float angleDY) noexcept;

	// Move floating camera in Right-Left axis (dX < 0 - movement left, dX > 0 - movement right)
	void MoveFloatingCameraX(Storage& registry, EID camera, float dX) noexcept;
	// Move floating camera in Up-Down axis (dY < 0 - movement down, dY > 0 - movement up)
	void MoveFloatingCameraY(Storage& registry, EID camera, float dY) noexcept;
	// Move floating camera in Forward-Backward axis (dZ < 0 - movement backward, dZ > 0 - movement forward)
	void MoveFloatingCameraZ(Storage& registry, EID camera, float dZ) noexcept;
	// Roll floating camera view in Left-Right direction (delta < 0 - roll counterclockwise, delta > 0 roll clockwise)
	void RollFloatingCamera(Storage& registry, EID camera, float delta) noexcept;
	// Rotate floating camera view (angleDX < 0 - rotate down, angleDX > 0 - rotate up; angleDY < 0 rotate left, angleDY > 0 - rotate right)
	void RotateFloatingCamera(Storage& registry, EID camera, float angleDX, float angleDY) noexcept;
#pragma endregion
}
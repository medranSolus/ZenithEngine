#pragma once
#include "Entity.h"
#include "Settings.h"

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
	// Assure that all camera components are registered as pools in data storage
	constexpr void InitCameraComponents() noexcept { Settings::AssureEntityPools<Camera>(); }

	// Convert projection space jitter into unit pixel space (range [-0.5,0.5]) in X axis
	constexpr float GetUnitPixelJitterX(float jitterX, U32 renderWidth) noexcept { return 0.5f * jitterX * Utils::SafeCast<float>(renderWidth); }
	// Convert projection space jitter into unit pixel space (range [-0.5,0.5]) in Y axis
	constexpr float GetUnitPixelJitterY(float jitterY, U32 renderHeight) noexcept { return -0.5f * jitterY * Utils::SafeCast<float>(renderHeight); }

	// Get frustum from infinite far plan reversed clip space projection
	Math::BoundingFrustum GetFrustum(Matrix proj, float maxDistance) noexcept;
	// Get symetrical vertical FOV projection matrix with infinite far plane for reversed clip space
	Matrix GetProjectionMatrix(const Projection& proj) noexcept;
	// Move any camera in Right-Left axis (dX < 0 - movement left, dX > 0 - movement right)
	void MoveCameraX(EID camera, float dX, CameraType type) noexcept;
	// Move any camera in Up-Down axis (dY < 0 - movement down, dY > 0 - movement up)
	void MoveCameraY(EID camera, float dY, CameraType type) noexcept;
	// Move any camera in Forward-Backward axis (dZ < 0 - movement backward, dZ > 0 - movement forward)
	void MoveCameraZ(EID camera, float dZ, CameraType type) noexcept;
	// Roll any camera view in Left-Right direction (delta < 0 - roll counterclockwise, delta > 0 roll clockwise)
	void RollCamera(EID camera, float delta, CameraType type) noexcept;
	// Rotate any camera view (angleDX < 0 - rotate down, angleDX > 0 - rotate up; angleDY < 0 rotate left, angleDY > 0 - rotate right)
	void RotateCamera(EID camera, float angleDX, float angleDY, CameraType type) noexcept;

	// Move person camera in Right-Left axis (dX < 0 - movement left, dX > 0 - movement right)
	void MovePersonCameraX(EID camera, float dX) noexcept;
	// Move person camera in Up-Down axis (dY < 0 - movement down, dY > 0 - movement up)
	void MovePersonCameraY(EID camera, float dY) noexcept;
	// Move person camera in Forward-Backward axis (dZ < 0 - movement backward, dZ > 0 - movement forward)
	void MovePersonCameraZ(EID camera, float dZ) noexcept;
	// Roll person camera view in Left-Right direction (delta < 0 - roll counterclockwise, delta > 0 roll clockwise)
	void RollPersonCamera(EID camera, float delta) noexcept;
	// Rotate person camera view (angleDX < 0 - rotate down, angleDX > 0 - rotate up; angleDY < 0 rotate left, angleDY > 0 - rotate right)
	void RotatePersonCamera(EID camera, float angleDX, float angleDY) noexcept;

	// Move floating camera in Right-Left axis (dX < 0 - movement left, dX > 0 - movement right)
	void MoveFloatingCameraX(EID camera, float dX) noexcept;
	// Move floating camera in Up-Down axis (dY < 0 - movement down, dY > 0 - movement up)
	void MoveFloatingCameraY(EID camera, float dY) noexcept;
	// Move floating camera in Forward-Backward axis (dZ < 0 - movement backward, dZ > 0 - movement forward)
	void MoveFloatingCameraZ(EID camera, float dZ) noexcept;
	// Roll floating camera view in Left-Right direction (delta < 0 - roll counterclockwise, delta > 0 roll clockwise)
	void RollFloatingCamera(EID camera, float delta) noexcept;
	// Rotate floating camera view (angleDX < 0 - rotate down, angleDX > 0 - rotate up; angleDY < 0 rotate left, angleDY > 0 - rotate right)
	void RotateFloatingCamera(EID camera, float angleDX, float angleDY) noexcept;
#pragma endregion
}
#pragma once
#include "Camera.h"
#include "Entity.h"

namespace ZE::Data
{
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
}
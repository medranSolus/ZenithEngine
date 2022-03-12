#include "Data/Operations.h"
#include "Data/Transform.h"

namespace ZE::Data
{
	void MoveCameraX(Storage& registry, EID camera, float dX, CameraType type) noexcept
	{
		switch (type)
		{
		default:
			ZE_ASSERT(false, "Unhandled enum value!");
		case CameraType::Person:
			return MovePersonCameraX(registry, camera, dX);
		case CameraType::Floating:
			return MoveFloatingCameraX(registry, camera, dX);
		}
	}

	void MoveCameraY(Storage& registry, EID camera, float dY, CameraType type) noexcept
	{
		switch (type)
		{
		default:
			ZE_ASSERT(false, "Unhandled enum value!");
		case CameraType::Person:
			return MovePersonCameraY(registry, camera, dY);
		case CameraType::Floating:
			return MoveFloatingCameraY(registry, camera, dY);
		}
	}

	void MoveCameraZ(Storage& registry, EID camera, float dZ, CameraType type) noexcept
	{
		switch (type)
		{
		default:
			ZE_ASSERT(false, "Unhandled enum value!");
		case CameraType::Person:
			return MovePersonCameraZ(registry, camera, dZ);
		case CameraType::Floating:
			return MoveFloatingCameraZ(registry, camera, dZ);
		}
	}

	void RollCamera(Storage& registry, EID camera, float delta, CameraType type) noexcept
	{
		switch (type)
		{
		default:
			ZE_ASSERT(false, "Unhandled enum value!");
		case CameraType::Person:
			return RollPersonCamera(registry, camera, delta);
		case CameraType::Floating:
			return RollFloatingCamera(registry, camera, delta);
		}
	}

	void RotateCamera(Storage& registry, EID camera, float angleDX, float angleDY, CameraType type) noexcept
	{
		switch (type)
		{
		default:
			ZE_ASSERT(false, "Unhandled enum value!");
		case CameraType::Person:
			return RotatePersonCamera(registry, camera, angleDX, angleDY);
		case CameraType::Floating:
			return RotateFloatingCamera(registry, camera, angleDX, angleDY);
		}
	}

	void MovePersonCameraX(Storage& registry, EID camera, float dX) noexcept
	{
		Float3& position = registry.get<Transform>(camera).Position;
		Float3 moveDir = registry.get<Camera>(camera).EyeDirection;
		moveDir.y = 0.0f;

		Math::XMStoreFloat3(&position,
			Math::XMVectorAdd(Math::XMLoadFloat3(&position),
				Math::XMVectorScale(Math::XMVector3Cross({ 0.0f, 1.0f, 0.0f, 0.0f },
					Math::XMVector3Normalize(Math::XMLoadFloat3(&moveDir))), dX)));
	}

	void MovePersonCameraY(Storage& registry, EID camera, float dY) noexcept
	{
		registry.get<Transform>(camera).Position.y += dY;
	}

	void MovePersonCameraZ(Storage& registry, EID camera, float dZ) noexcept
	{
		Float3& position = registry.get<Transform>(camera).Position;
		Float3 moveDir = registry.get<Camera>(camera).EyeDirection;
		moveDir.y = 0.0f;

		Math::XMStoreFloat3(&position,
			Math::XMVectorAdd(Math::XMLoadFloat3(&position),
				Math::XMVectorScale(Math::XMVector3Normalize(Math::XMLoadFloat3(&moveDir)), dZ)));
	}

	void RollPersonCamera(Storage& registry, EID camera, float delta) noexcept
	{
		Float3 moveDir = registry.get<Camera>(camera).EyeDirection;
		moveDir.y = 0.0f;

		const Vector rotor = Math::XMQuaternionRotationNormal(Math::XMVector3Normalize(Math::XMLoadFloat3(&moveDir)), delta);

		Float3& up = registry.get<Camera>(camera).UpVector;
		Math::XMStoreFloat3(&up, Math::XMVector3Rotate(Math::XMLoadFloat3(&up), rotor));

		Float4& rotation = registry.get<Transform>(camera).Rotation;
		ZE_ASSERT_Q_UNIT(rotation);
		Math::XMStoreFloat4(&rotation,
			Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(Math::XMLoadFloat4(&rotation), rotor)));
		ZE_ASSERT_Q_UNIT(rotation);
	}

	void RotatePersonCamera(Storage& registry, EID camera, float angleDX, float angleDY) noexcept
	{
		Camera& cam = registry.get<Camera>(camera);

		if (abs(angleDX) < Camera::ROTATE_EPSILON)
			angleDX = 0.0f;
		if (abs(angleDY) < Camera::ROTATE_EPSILON)
		{
			angleDY = 0.0f;
			if (angleDX == 0.0f)
				return;
		}

		const Vector upV = Math::XMLoadFloat3(&cam.UpVector);
		const Vector eyeDirV = Math::XMLoadFloat3(&cam.EyeDirection);
		Vector rotor = {};
		if (angleDX)
		{
			const float angle = Math::XMVectorGetX(Math::XMVector3AngleBetweenNormals(upV, eyeDirV)) + angleDX;
			if (angle <= Camera::FLIP_EPSILON || angle >= M_PI - Camera::FLIP_EPSILON)
			{
				angleDX = 0.0f;
				if (angleDY == 0.0f)
					return;
			}
			else
			{
				Float3 moveDir = cam.EyeDirection;
				moveDir.y = 0.0f;

				rotor = Math::XMQuaternionRotationNormal(Math::XMVector3Cross(upV,
					Math::XMVector3Normalize(Math::XMLoadFloat3(&moveDir))), angleDX);
			}
		}
		if (angleDY)
		{
			const Vector rotorY = Math::XMQuaternionRotationNormal(upV, angleDY);
			rotor = angleDX != 0.0f ? Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(rotor, rotorY)) : rotorY;
		}

		// Unknown rotation when UP is strongly tilted, TODO: Perform some tests
		Math::XMStoreFloat3(&cam.EyeDirection, Math::XMVector3Normalize(Math::XMVector3Rotate(eyeDirV, rotor)));

		Float4& rotation = registry.get<Transform>(camera).Rotation;
		ZE_ASSERT_Q_UNIT(rotation);
		Math::XMStoreFloat4(&rotation,
			Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(Math::XMLoadFloat4(&rotation), rotor)));
		ZE_ASSERT_Q_UNIT(rotation);
	}

	void MoveFloatingCameraX(Storage& registry, EID camera, float dX) noexcept
	{
		Float3& position = registry.get<Transform>(camera).Position;
		const Camera& cam = registry.get<Camera>(camera);

		Math::XMStoreFloat3(&position,
			Math::XMVectorAdd(Math::XMLoadFloat3(&position),
				Math::XMVectorScale(Math::XMVector3Cross(Math::XMLoadFloat3(&cam.UpVector),
					Math::XMLoadFloat3(&cam.EyeDirection)), dX)));
	}

	void MoveFloatingCameraY(Storage& registry, EID camera, float dY) noexcept
	{
		Float3& position = registry.get<Transform>(camera).Position;
		const Float3& up = registry.get<Camera>(camera).UpVector;

		Math::XMStoreFloat3(&position,
			Math::XMVectorAdd(Math::XMLoadFloat3(&position),
				Math::XMVectorScale(Math::XMLoadFloat3(&up), dY)));
	}

	void MoveFloatingCameraZ(Storage& registry, EID camera, float dZ) noexcept
	{
		Float3& position = registry.get<Transform>(camera).Position;
		const Float3& moveDir = registry.get<Camera>(camera).EyeDirection;

		Math::XMStoreFloat3(&position,
			Math::XMVectorAdd(Math::XMLoadFloat3(&position),
				Math::XMVectorScale(Math::XMLoadFloat3(&moveDir), dZ)));
	}

	void RollFloatingCamera(Storage& registry, EID camera, float delta) noexcept
	{
		const Vector rotor = Math::XMQuaternionRotationNormal(Math::XMLoadFloat3(&registry.get<Camera>(camera).EyeDirection), delta);

		Float3& up = registry.get<Camera>(camera).UpVector;
		Math::XMStoreFloat3(&up, Math::XMVector3Rotate(Math::XMLoadFloat3(&up), rotor));

		Float4& rotation = registry.get<Transform>(camera).Rotation;
		ZE_ASSERT_Q_UNIT(rotation);
		Math::XMStoreFloat4(&rotation,
			Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(Math::XMLoadFloat4(&rotation), rotor)));
		ZE_ASSERT_Q_UNIT(rotation);
	}

	void RotateFloatingCamera(Storage& registry, EID camera, float angleDX, float angleDY) noexcept
	{
		Camera& cam = registry.get<Camera>(camera);

		if (abs(angleDX) < Camera::ROTATE_EPSILON)
			angleDX = 0.0f;
		if (abs(angleDY) < Camera::ROTATE_EPSILON)
		{
			angleDY = 0.0f;
			if (angleDX == 0.0f)
				return;
		}

		const Vector upV = Math::XMLoadFloat3(&cam.UpVector);
		const Vector moveDirV = Math::XMLoadFloat3(&cam.EyeDirection);
		Vector rotor = {};
		if (angleDX)
		{
			rotor = Math::XMQuaternionRotationNormal(Math::XMVector3Cross(upV, moveDirV), angleDX);
			Math::XMStoreFloat3(&cam.UpVector, Math::XMVector3Normalize(Math::XMVector3Rotate(upV, rotor)));
		}
		if (angleDY)
		{
			const Vector rotorY = Math::XMQuaternionRotationNormal(upV, angleDY);
			rotor = angleDX != 0.0f ? Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(rotor, rotorY)) : rotorY;
		}
		Math::XMStoreFloat3(&cam.EyeDirection,
			Math::XMVector3Normalize(Math::XMVector3Rotate(moveDirV, rotor)));

		Float4& rotation = registry.get<Transform>(camera).Rotation;
		ZE_ASSERT_Q_UNIT(rotation);
		Math::XMStoreFloat4(&rotation,
			Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(Math::XMLoadFloat4(&rotation), rotor)));
		ZE_ASSERT_Q_UNIT(rotation);
	}

	void UpdateTransforms() noexcept
	{
		//ZE_ASSERT(EntityInfo.Size == 0 || Entities[EntityInfo.Size - 1].ParentID == Data::Entity::INVALID_ID,
		//	"Last entity cannot be child enitty!");

		//// Update global transforms of every root entity
		//EID i = TransformInfo.Size;
		//while (i)
		//{
		//	Data::Entity entity = GetEntity(TransformEntities[--i]);
		//	if (entity.ParentID != Data::Entity::INVALID_ID)
		//		break;
		//	TransformsGlobal[i] = TransformsLocal[i];
		//}

		//// Update every child entity
		//while (i)
		//{
		//	EID parentID = GetEntity(TransformEntities[--i]).ParentID;
		//	ZE_ASSERT(parentID != Data::Entity::INVALID_ID, "Root entities should be already processed!");

		//	const Data::Transform& parent = TransformsGlobal[TransformPositions.at(parentID)];
		//	const Data::Transform& local = TransformsLocal[i];
		//	Data::Transform& global = TransformsGlobal[i];

		//	Math::XMStoreFloat4(&global.Rotation,
		//		Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(
		//			Math::XMLoadFloat4(&parent.Rotation),
		//			Math::XMLoadFloat4(&local.Rotation))));
		//	Math::XMStoreFloat3(&global.Position,
		//		Math::XMVectorAdd(
		//			Math::XMLoadFloat3(&global.Position),
		//			Math::XMLoadFloat3(&local.Position)));
		//	Math::XMStoreFloat3(&global.Scale,
		//		Math::XMVectorMultiply(
		//			Math::XMLoadFloat3(&global.Scale),
		//			Math::XMLoadFloat3(&local.Scale)));
		//}
	}
}
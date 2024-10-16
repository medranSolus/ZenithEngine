#include "Data/Camera.h"
#include "Data/Transform.h"
#include "Settings.h"

namespace ZE::Data
{
	Math::BoundingFrustum GetFrustum(Matrix proj, float maxDistance) noexcept
	{
		// Re-reverse near and far planes for correct frustum tests and disable infinite depth
#if defined(_XM_NO_INTRINSICS_)
		proj.m[2][2] = maxDistance / (maxDistance - proj.m[3][2]);
		proj.m[3][2] *= -proj.m[2][2];
#elif defined(_XM_ARM_NEON_INTRINSICS_)
		const float nearClip = Math::XMVectorGetZ(proj.r[3]);
		const float fRange = maxDistance / (maxDistance - nearClip);

		const float32x4_t zero = vdupq_n_f32(0);
		proj.r[2] = vsetq_lane_f32(fRange, Math::g_XMIdentityR3.v, 2);
		proj.r[3] = vsetq_lane_f32(-fRange * nearClip, zero, 2);
#elif defined(_XM_SSE_INTRINSICS_)
		const float nearClip = Math::XMVectorGetZ(proj.r[3]);
		const float fRange = maxDistance / (maxDistance - nearClip);
		// Note: This is recorded on the stack
		Vector rMem = { 0.0f, 0.0f, fRange, -fRange * nearClip };
		// Copy from memory to SSE register
		Vector vValues = rMem;
		Vector vTemp = _mm_setzero_ps();
		vValues = _mm_shuffle_ps(vValues, Math::g_XMIdentityR3, _MM_SHUFFLE(3, 2, 3, 2));
		// 0,0,fRange,1.0f
		vTemp = _mm_shuffle_ps(vTemp, vValues, _MM_SHUFFLE(3, 0, 0, 0));
		proj.r[2] = vTemp;
		// 0,0,nearRange,0.0f
		vTemp = _mm_shuffle_ps(vTemp, vValues, _MM_SHUFFLE(2, 1, 0, 0));
		proj.r[3] = vTemp;
#endif
		return Math::BoundingFrustum{ proj, false };
	}

	Matrix GetProjectionMatrix(const Projection& proj) noexcept
	{
		constexpr float F_RANGE = 0.0f;
		// Based on XMMatrixPerspectiveFovLH
		ZE_ASSERT(proj.NearClip > 0.0f, "Near Z must be greater than 0!");
		ZE_ASSERT(!Math::XMScalarNearEqual(proj.FOV, 0.0f, 0.00001f * 2.0f), "FOV must be greater than 0!");
		ZE_ASSERT(!Math::XMScalarNearEqual(proj.ViewRatio, 0.0f, 0.00001f), "Aspect ration must be greater than 0!");

		float sinFov, cosFov;
		Math::XMScalarSinCos(&sinFov, &cosFov, 0.5f * proj.FOV);
		const float height = cosFov / sinFov;
		const float width = height / proj.ViewRatio;
		const float nearRange = proj.NearClip;
		float jitterX = 0.0f;
		float jitterY = 0.0f;
		if (Settings::ApplyJitter())
		{
			jitterX = proj.JitterX;
			jitterY = proj.JitterY;
		}

		Matrix m;
#if defined(_XM_NO_INTRINSICS_)
		m.m[0][0] = width;
		m.m[0][1] = 0.0f;
		m.m[0][2] = 0.0f;
		m.m[0][3] = 0.0f;

		m.m[1][0] = 0.0f;
		m.m[1][1] = height;
		m.m[1][2] = 0.0f;
		m.m[1][3] = 0.0f;

		m.m[2][0] = jitterX;
		m.m[2][1] = jitterY;
		m.m[2][2] = F_RANGE;
		m.m[2][3] = 1.0f;

		m.m[3][0] = 0.0f;
		m.m[3][1] = 0.0f;
		m.m[3][2] = nearRange;
		m.m[3][3] = 0.0f;
#elif defined(_XM_ARM_NEON_INTRINSICS_)
		const float32x4_t zero = vdupq_n_f32(0);

		m.r[0] = vsetq_lane_f32(width, zero, 0);
		m.r[1] = vsetq_lane_f32(height, zero, 1);
		if (Settings::ApplyJitter())
			m.r[2] = Math::XMVectorSet(jitterX, jitterY, F_RANGE, 1.0f);
		else
			m.r[2] = vsetq_lane_f32(F_RANGE, Math::g_XMIdentityR3.v, 2);
		m.r[3] = vsetq_lane_f32(nearRange, zero, 2);
#elif defined(_XM_SSE_INTRINSICS_)
		// Note: This is recorded on the stack
		Vector rMem = { width, height, F_RANGE, nearRange };
		// Copy from memory to SSE register
		Vector vValues = rMem;
		Vector vTemp = _mm_setzero_ps();
		// Copy x only
		vTemp = _mm_move_ss(vTemp, vValues);
		// width,0,0,0
		m.r[0] = vTemp;
		// 0,height,0,0
		vTemp = vValues;
		vTemp = _mm_and_ps(vTemp, Math::g_XMMaskY);
		m.r[1] = vTemp;
		// x=F_RANGE,y=nearRange,0,1.0f
		vTemp = _mm_setzero_ps();
		vValues = _mm_shuffle_ps(vValues, Math::g_XMIdentityR3, _MM_SHUFFLE(3, 2, 3, 2));
		// 0,0,F_RANGE,1.0f
		vTemp = _mm_shuffle_ps(vTemp, vValues, _MM_SHUFFLE(3, 0, 0, 0));

		if (Settings::ApplyJitter())
			m.r[2] = Math::XMVectorSet(jitterX, jitterY, F_RANGE, 1.0f);
		else
			m.r[2] = vTemp;

		// 0,0,nearRange,0.0f
		vTemp = _mm_shuffle_ps(vTemp, vValues, _MM_SHUFFLE(2, 1, 0, 0));
		m.r[3] = vTemp;
#endif
		return m;
	}

	void MoveCameraX(EID camera, float dX, CameraType type) noexcept
	{
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case CameraType::Person:
			return MovePersonCameraX(camera, dX);
		case CameraType::Floating:
			return MoveFloatingCameraX(camera, dX);
		}
	}

	void MoveCameraY(EID camera, float dY, CameraType type) noexcept
	{
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case CameraType::Person:
			return MovePersonCameraY(camera, dY);
		case CameraType::Floating:
			return MoveFloatingCameraY(camera, dY);
		}
	}

	void MoveCameraZ(EID camera, float dZ, CameraType type) noexcept
	{
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case CameraType::Person:
			return MovePersonCameraZ(camera, dZ);
		case CameraType::Floating:
			return MoveFloatingCameraZ(camera, dZ);
		}
	}

	void RollCamera(EID camera, float delta, CameraType type) noexcept
	{
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case CameraType::Person:
			return RollPersonCamera(camera, delta);
		case CameraType::Floating:
			return RollFloatingCamera(camera, delta);
		}
	}

	void RotateCamera(EID camera, float angleDX, float angleDY, CameraType type) noexcept
	{
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case CameraType::Person:
			return RotatePersonCamera(camera, angleDX, angleDY);
		case CameraType::Floating:
			return RotateFloatingCamera(camera, angleDX, angleDY);
		}
	}

	void MovePersonCameraX(EID camera, float dX) noexcept
	{
		ZE_VALID_EID(camera);

		Float3& position = Settings::Data.get<Transform>(camera).Position;
		Float3 moveDir = Settings::Data.get<Camera>(camera).EyeDirection;
		moveDir.y = 0.0f;

		Math::XMStoreFloat3(&position,
			Math::XMVectorAdd(Math::XMLoadFloat3(&position),
				Math::XMVectorScale(Math::XMVector3Cross({ 0.0f, 1.0f, 0.0f, 0.0f },
					Math::XMVector3Normalize(Math::XMLoadFloat3(&moveDir))), dX)));
	}

	void MovePersonCameraY(EID camera, float dY) noexcept
	{
		ZE_VALID_EID(camera);
		Settings::Data.get<Transform>(camera).Position.y += dY;
	}

	void MovePersonCameraZ(EID camera, float dZ) noexcept
	{
		ZE_VALID_EID(camera);

		Float3& position = Settings::Data.get<Transform>(camera).Position;
		Float3 moveDir = Settings::Data.get<Camera>(camera).EyeDirection;
		moveDir.y = 0.0f;

		Math::XMStoreFloat3(&position,
			Math::XMVectorAdd(Math::XMLoadFloat3(&position),
				Math::XMVectorScale(Math::XMVector3Normalize(Math::XMLoadFloat3(&moveDir)), dZ)));
	}

	void RollPersonCamera(EID camera, float delta) noexcept
	{
		ZE_VALID_EID(camera);

		Float3 moveDir = Settings::Data.get<Camera>(camera).EyeDirection;
		moveDir.y = 0.0f;

		const Vector rotor = Math::XMQuaternionRotationNormal(Math::XMVector3Normalize(Math::XMLoadFloat3(&moveDir)), delta);

		Float3& up = Settings::Data.get<Camera>(camera).UpVector;
		Math::XMStoreFloat3(&up, Math::XMVector3Rotate(Math::XMLoadFloat3(&up), rotor));

		Float4& rotation = Settings::Data.get<Transform>(camera).Rotation;
		ZE_ASSERT_Q_UNIT(rotation);
		Math::XMStoreFloat4(&rotation,
			Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(Math::XMLoadFloat4(&rotation), rotor)));
		ZE_ASSERT_Q_UNIT(rotation);
	}

	void RotatePersonCamera(EID camera, float angleDX, float angleDY) noexcept
	{
		ZE_VALID_EID(camera);
		Camera& cam = Settings::Data.get<Camera>(camera);

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

		Float4& rotation = Settings::Data.get<Transform>(camera).Rotation;
		ZE_ASSERT_Q_UNIT(rotation);
		Math::XMStoreFloat4(&rotation,
			Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(Math::XMLoadFloat4(&rotation), rotor)));
		ZE_ASSERT_Q_UNIT(rotation);
	}

	void MoveFloatingCameraX(EID camera, float dX) noexcept
	{
		ZE_VALID_EID(camera);

		Float3& position = Settings::Data.get<Transform>(camera).Position;
		const Camera& cam = Settings::Data.get<Camera>(camera);

		Math::XMStoreFloat3(&position,
			Math::XMVectorAdd(Math::XMLoadFloat3(&position),
				Math::XMVectorScale(Math::XMVector3Cross(Math::XMLoadFloat3(&cam.UpVector),
					Math::XMLoadFloat3(&cam.EyeDirection)), dX)));
	}

	void MoveFloatingCameraY(EID camera, float dY) noexcept
	{
		ZE_VALID_EID(camera);

		Float3& position = Settings::Data.get<Transform>(camera).Position;
		const Float3& up = Settings::Data.get<Camera>(camera).UpVector;

		Math::XMStoreFloat3(&position,
			Math::XMVectorAdd(Math::XMLoadFloat3(&position),
				Math::XMVectorScale(Math::XMLoadFloat3(&up), dY)));
	}

	void MoveFloatingCameraZ(EID camera, float dZ) noexcept
	{
		ZE_VALID_EID(camera);

		Float3& position = Settings::Data.get<Transform>(camera).Position;
		const Float3& moveDir = Settings::Data.get<Camera>(camera).EyeDirection;

		Math::XMStoreFloat3(&position,
			Math::XMVectorAdd(Math::XMLoadFloat3(&position),
				Math::XMVectorScale(Math::XMLoadFloat3(&moveDir), dZ)));
	}

	void RollFloatingCamera(EID camera, float delta) noexcept
	{
		ZE_VALID_EID(camera);
		const Vector rotor = Math::XMQuaternionRotationNormal(Math::XMLoadFloat3(&Settings::Data.get<Camera>(camera).EyeDirection), delta);

		Float3& up = Settings::Data.get<Camera>(camera).UpVector;
		Math::XMStoreFloat3(&up, Math::XMVector3Rotate(Math::XMLoadFloat3(&up), rotor));

		Float4& rotation = Settings::Data.get<Transform>(camera).Rotation;
		ZE_ASSERT_Q_UNIT(rotation);
		Math::XMStoreFloat4(&rotation,
			Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(Math::XMLoadFloat4(&rotation), rotor)));
		ZE_ASSERT_Q_UNIT(rotation);
	}

	void RotateFloatingCamera(EID camera, float angleDX, float angleDY) noexcept
	{
		ZE_VALID_EID(camera);
		Camera& cam = Settings::Data.get<Camera>(camera);

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

		Float4& rotation = Settings::Data.get<Transform>(camera).Rotation;
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
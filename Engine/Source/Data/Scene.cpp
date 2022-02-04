#include "Data/Scene.h"

namespace ZE::Data
{
	EID Scene::CreateEntity() noexcept
	{
		Entity entity = {};
		EID id = EntityInfo.Size;
		while (EntityPositions.contains(id))
			++id;
		EntityPositions.emplace(id, EntityInfo.Size);
		if (Entities == nullptr)
			Entities = Table::Create(1U, Entity(id));
		else
			Table::Append<1>(EntityInfo, Entities, id);
		return id;
	}

	void Scene::AddCamera(EID entity, const Camera& camera) noexcept
	{
		ZE_ASSERT(!CameraPositions.contains(entity), "Entity already contains Camera component!");

		CameraPositions.emplace(entity, CameraInfo.Size);
		if (Cameras == nullptr)
			Cameras = Table::Create(1U, camera);
		else
			Table::Append<1>(CameraInfo, Cameras, camera);
	}

	void Scene::UpdateTransforms() noexcept
	{
		ZE_ASSERT(EntityInfo.Size == 0 || Entities[EntityInfo.Size - 1].ParentID == Data::Entity::INVALID_ID,
			"Last entity cannot be child enitty!");

		// Update global transforms of every root entity
		EID i = TransformInfo.Size;
		while (i)
		{
			Data::Entity entity = GetEntity(TransformEntities[--i]);
			if (entity.ParentID != Data::Entity::INVALID_ID)
				break;
			TransformsGlobal[i] = TransformsLocal[i];
		}

		// Update every child entity
		while (i)
		{
			EID parentID = GetEntity(TransformEntities[--i]).ParentID;
			ZE_ASSERT(parentID != Data::Entity::INVALID_ID, "Root entities should be already processed!");

			const Data::Transform& parent = TransformsGlobal[TransformPositions.at(parentID)];
			const Data::Transform& local = TransformsLocal[i];
			Data::Transform& global = TransformsGlobal[i];

			Math::XMStoreFloat4(&global.Rotation,
				Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(
					Math::XMLoadFloat4(&parent.Rotation),
					Math::XMLoadFloat4(&local.Rotation))));
			Math::XMStoreFloat3(&global.Position,
				Math::XMVectorAdd(
					Math::XMLoadFloat3(&global.Position),
					Math::XMLoadFloat3(&local.Position)));
			Math::XMStoreFloat3(&global.Scale,
				Math::XMVectorMultiply(
					Math::XMLoadFloat3(&global.Scale),
					Math::XMLoadFloat3(&local.Scale)));
		}
	}
}
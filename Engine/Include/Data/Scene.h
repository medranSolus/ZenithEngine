#pragma once
#include "Camera.h"
#include "Entity.h"
#include "Geometry.h"
#include "MaterialPBR.h"
#include "Mesh.h"
#include "Model.h"
#include "Transform.h"

namespace ZE::Data
{
	// Hashmap containing info about exact position of element in table
	template<typename ID>
	using LocationLookup = std::unordered_map<ID, U64>;

	// Main structure holding info about all the components in the scene
	struct Scene
	{
		TableInfo<EID> EntityInfo;
		Ptr<Entity> Entities;
		LocationLookup<EID> EntityPositions;

		// Ordering by non-decreasing parent, root entities at the end.
		// If child entity have Transform then all parents should have said component too
		TableInfo<EID> TransformInfo;
		Ptr<EID> TransformEntities;
		Ptr<Transform> TransformsLocal;
		Ptr<Transform> TransformsGlobal;
		LocationLookup<EID> TransformPositions;

		TableInfo<EID> ModelInfo;
		Ptr<EID> ModelEntities;
		Ptr<Model> Models;
		LocationLookup<EID> ModelPositions;

		TableInfo<U64> MeshInfo;
		Ptr<Mesh> Meshes;
		LocationLookup<U64> MeshPositions;

		TableInfo<U64> GeometryInfo;
		Ptr<Geometry> Geometries;

		TableInfo<U64> MaterialInfo;
		Ptr<MaterialPBR> Materials;

		TableInfo<EID> CameraInfo;
		Ptr<Camera> Cameras;
		LocationLookup<EID> CameraPositions;

		Entity GetEntity(EID id) const noexcept { ZE_ASSERT(id != Entity::INVALID_ID, "Invalid ID!"); return Entities[EntityPositions.at(id)]; }

		// Maybe move out of Scene as independent functions
		EID CreateEntity() noexcept;
		void AddCamera(EID entity, const Camera& camera) noexcept;
		void UpdateTransforms() noexcept;
	};
}
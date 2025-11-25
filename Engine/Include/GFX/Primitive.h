#pragma once
#include "Resource/MeshData.h"
#include "Data/Camera.h"
#include "Vertex.h"

namespace ZE::GFX::Primitive
{
	// Generic mesh data
	template<typename V, typename I = U32>
	struct Data
	{
		std::vector<V> Vertices;
		std::vector<I> Indices;
	};

	// Convert generic mesh data into format accepted by GPU resorces
	template<typename V, typename I, U64 DEST_INDEX_SIZE = sizeof(I)>
	constexpr std::shared_ptr<U8[]> GetPackedMesh(const std::vector<V>& vertices, const std::vector<I>& indices) noexcept;

	// Wrapper over standard GetPackedMesh to check if the mesh data can be packed into smaller index size
	template<typename V, typename I>
	constexpr std::shared_ptr<U8[]> GetPackedMeshPackIndex(const std::vector<V>& vertices, const std::vector<I>& indices, U8& resultingIndexSize) noexcept;

	// Compute simple normals based on normal vector of the surface and tangent vectors
	void ComputeSurfaceNormalsTangents(std::vector<Vertex>& vertices, const std::vector<U32>& indices) noexcept;
	// Compute simple normals based on normal vector of the surface and tangent vectors
	void ComputeSurfaceNormalsTangents(std::vector<Vertex>& vertices, const std::vector<U16>& indices) noexcept;
	// Computes tangent vectors for mesh. Must contain valid UV and normal vectors
	void ComputeTangents(std::vector<Vertex>& vertices, const std::vector<U32>& indices) noexcept;
	// Computes tangent vectors for mesh. Must contain valid UV and normal vectors
	void ComputeTangents(std::vector<Vertex>& vertices, const std::vector<U16>& indices) noexcept;

	namespace Square
	{
		// Generate bounding box for simple square
		constexpr Math::BoundingBox MakeBoundingBox() noexcept { return { { 0.0f, 0.0f, 0.0f }, { 0.5f, 0.5f, 0.0f } }; }

		// Generate unlit square vertex data
		std::vector<Float3> MakeSolidVertex() noexcept;
		// Generate unlit square index data
		std::vector<U16> MakeSolidIndex() noexcept;
	}
	namespace Cube
	{
		// Generate bounding box for any cube
		constexpr Math::BoundingBox MakeBoundingBox() noexcept { return { { 0.0f, 0.0f, 0.0f }, { 0.5f, 0.5f, 0.5f } }; }

		// Generate unlit cube vertex data
		std::vector<Float3> MakeSolidVertex() noexcept;
		// Generate unlit cube index data
		std::vector<U16> MakeSolidIndex() noexcept;
		// Generate unlit cube index data (front face inside cube)
		std::vector<U16> MakeSolidIndexInverted() noexcept;

		// Generate cube vertex data (requires index data from MakeIndex())
		std::vector<Vertex> MakeVertex(const std::vector<U16>& indices) noexcept;
		// Generate cube index data
		std::vector<U16> MakeIndex() noexcept;
	}
	namespace Cone
	{
		// Generate bounding box for camera indicator
		constexpr Math::BoundingBox MakeBoundingBox() noexcept { return { { 0.0f, 0.5f, 0.0f }, { 0.5f, 0.5f, 0.5f } }; }

		// Generate unlit cone vertex and index data
		Data<Float3, U16> MakeSolid(U16 density) noexcept;
	}
	namespace CameraFrame
	{
		// Generate bounding box for camera indicator
		constexpr Math::BoundingBox MakeIndicatorBoundingBox() noexcept { return { { 0.0f, 0.35f, 0.5f }, { 1.0f, 1.05f, 0.5f } }; }

		// Generate camera indicator wireframe vertex data
		std::vector<Float3> MakeIndicatorVertex() noexcept;
		// Generate camera indicator wireframe index data
		std::vector<U16> MakeIndicatorIndex() noexcept;

		// Generate bounding box for camera frustum
		Math::BoundingBox MakeFrustumBoundingBox(const ZE::Data::Projection& projection, float farClip = FLT_MAX) noexcept;

		// Generate camera frustum wireframe vertex data
		std::vector<Float3> MakeFrustumVertex(const ZE::Data::Projection& projection, float farClip = FLT_MAX) noexcept;
		// Generate camera frustum wireframe index data
		std::vector<U16> MakeFrustumIndex() noexcept;
	}
	namespace Sphere
	{
		// Generate bounding box for any sphere
		constexpr Math::BoundingBox MakeBoundingBox() noexcept { return { { 0.0f, 0.0f, 0.0f }, { 0.5f, 0.5f, 0.5f } }; }

		// Generate unlit UV sphere vertex and index data, latitudeDensity: N-S, longitudeDensity: W-E
		Data<Float3> MakeUVSolid(U32 latitudeDensity, U32 longitudeDensity) noexcept;
		// Generate unlit Ico sphere vertex and index data
		Data<Float3> MakeIcoSolid(U32 density) noexcept;

		// Generate UV sphere vertex and index data, latitudeDensity: N-S, longitudeDensity: W-E
		Data<Vertex> MakeUV(U32 latitudeDensity, U32 longitudeDensity) noexcept;
		// Generate Ico sphere vertex and index data
		Data<Vertex> MakeIco(U32 density) noexcept;
	}

#pragma region Functions
	template<typename V, typename I, U64 DEST_INDEX_SIZE>
	constexpr std::shared_ptr<U8[]> GetPackedMesh(const std::vector<V>& vertices, const std::vector<I>& indices) noexcept
	{
		ZE_ASSERT(vertices.size(), "Empty vertex data!");
		ZE_ASSERT(indices.size(), "Empty index data!");

		// Pack into consecutive index and vertex arrays
		std::shared_ptr<U8[]> mesh = std::make_shared<U8[]>(indices.size() * DEST_INDEX_SIZE + vertices.size() * sizeof(V));
		const U64 vertexOffset = indices.size() * DEST_INDEX_SIZE;

		if constexpr (DEST_INDEX_SIZE == sizeof(I))
		{
			std::memcpy(mesh.get(), indices.data(), vertexOffset);
		}
		else
		{
			// Convert index size
			if constexpr (DEST_INDEX_SIZE == sizeof(U16))
			{
				U16* destIndices = reinterpret_cast<U16*>(mesh.get());
				for (U64 i = 0; i < indices.size(); ++i)
					destIndices[i] = Utils::SafeCast<U16>(indices.at(i));
			}
			else if constexpr (DEST_INDEX_SIZE == sizeof(U8))
			{
				U8* destIndices = reinterpret_cast<U8*>(mesh.get());
				for (U64 i = 0; i < indices.size(); ++i)
					destIndices[i] = Utils::SafeCast<U8>(indices.at(i));
			}
			else if constexpr (DEST_INDEX_SIZE == sizeof(U32))
			{
				U32* destIndices = reinterpret_cast<U32*>(mesh.get());
				for (U64 i = 0; i < indices.size(); ++i)
					destIndices[i] = Utils::SafeCast<U32>(indices.at(i));
			}
			else
			{
				static_assert(false, "Unsupported index size conversion!");
			}
		}
		std::memcpy(mesh.get() + Math::AlignUp(vertexOffset, static_cast<U64>(Resource::MeshData::VERTEX_BUFFER_ALIGNMENT)), vertices.data(), vertices.size() * sizeof(V));
		return mesh;
	}

	template<typename V, typename I>
	constexpr std::shared_ptr<U8[]> GetPackedMeshPackIndex(const std::vector<V>& vertices, const std::vector<I>& indices, U8& resultingIndexSize) noexcept
	{
		ZE_ASSERT(vertices.size(), "Empty vertex data!");
		ZE_ASSERT(indices.size(), "Empty index data!");

		switch (sizeof(I))
		{
		default:
			ZE_ENUM_UNHANDLED();
		case sizeof(U32):
		{
			if (vertices.size() >= UINT16_MAX)
			{
				resultingIndexSize = sizeof(U32);
				return GetPackedMesh<V, I, sizeof(U32)>(vertices, indices);
			}
			[[fallthrough]];
		}
		case sizeof(U16):
		{
			if (!Settings::IsEnabledU8IndexBuffers() || vertices.size() >= UINT8_MAX)
			{
				resultingIndexSize = sizeof(U16);
				return GetPackedMesh<V, I, sizeof(U16)>(vertices, indices);
			}
			[[fallthrough]];
		}
		case sizeof(U8):
		{
			if (Settings::IsEnabledU8IndexBuffers())
			{
				resultingIndexSize = sizeof(U8);
				return GetPackedMesh<V, I, sizeof(U8)>(vertices, indices);
			}
			resultingIndexSize = sizeof(U16);
			return GetPackedMesh<V, I, sizeof(U16)>(vertices, indices);
		}
		}
	}

#pragma endregion
}
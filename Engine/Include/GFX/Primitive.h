#pragma once
#include "Resource/MeshData.h"
#include "Vertex.h"

namespace ZE::GFX::Primitive
{
	// Generic mesh data
	template<typename T>
	struct Data
	{
		std::vector<T> Vertices;
		std::vector<U32> Indices;
	};

	// Convert generic mesh data into format accepted by GPU resorces
	template<typename V, typename I>
	constexpr std::shared_ptr<U8[]> GetPackedMesh(const std::vector<V>& vertices, const std::vector<I>& indices) noexcept;

	// Compute simple normals based on normal vector of the surface and tangent vectors
	void ComputeSurfaceNormalsTangents(std::vector<Vertex>& vertices, const std::vector<U32>& indices) noexcept;
	// Computes tangent vectors for mesh. Must contain valid UV and normal vectors
	void ComputeTangents(std::vector<Vertex>& vertices, const std::vector<U32>& indices) noexcept;

	// Generate bounding box for any cube
	constexpr Math::BoundingBox MakeCubeBoundingBox() noexcept { return { { 0.0f, 0.0f, 0.0f }, { 0.5, 0.5f, 0.5f } }; }

	// Generate simple cube vertex data
	std::vector<Float3> MakeCubeSolidVertex() noexcept;
	// Generate simple cube index data
	std::vector<U32> MakeCubeSolidIndex() noexcept;
	// Generate simple cube index data (front face inside cube)
	std::vector<U32> MakeCubeSolidIndexInverted() noexcept;

	// Generate cube vertex data (requires index data from MakeCubeIndex())
	std::vector<Vertex> MakeCubeVertex(const std::vector<U32>& indices) noexcept;
	// Generate cube index data
	std::vector<U32> MakeCubeIndex() noexcept;

	// Generate simple cone vertex and index data
	Data<Float3> MakeConeSolid(U32 density) noexcept;

	// Generate simple Ico sphere vertex and index data
	Data<Float3> MakeSphereIcoSolid(U32 density) noexcept;

#pragma region Functions
	template<typename V, typename I>
	constexpr std::shared_ptr<U8[]> GetPackedMesh(const std::vector<V>& vertices, const std::vector<I>& indices) noexcept
	{
		ZE_ASSERT(vertices.size(), "Empty vertex data!");
		ZE_ASSERT(indices.size(), "Empty index data!");

		// Pack into consecutive index and vertex arrays
		std::shared_ptr<U8[]> mesh = std::make_shared<U8[]>(indices.size() * sizeof(I) + vertices.size() * sizeof(V));
		const U64 vertexOffset = indices.size() * sizeof(I);
		std::memcpy(mesh.get(), indices.data(), vertexOffset);
		std::memcpy(mesh.get() + Math::AlignUp(vertexOffset, static_cast<U64>(Resource::MeshData::VERTEX_BUFFER_ALIGNMENT)), vertices.data(), vertices.size() * sizeof(V));
		return mesh;
	}
#pragma endregion
}
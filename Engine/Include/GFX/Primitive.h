#pragma once
#include "Vertex.h"

namespace ZE::GFX::Primitive
{
	template<typename T>
	struct Data
	{
		std::vector<T> Vertices;
		std::vector<U32> Indices;
	};

	// Compute simple normals based on normal vector of the surface and tangent vectors
	void ComputeSurfaceNormalsTangents(std::vector<Vertex>& vertices, const std::vector<U32>& indices) noexcept;
	// Computes tangent vectors for mesh. Must contain valid UV and normal vectors
	void ComputeTangents(std::vector<Vertex>& vertices, const std::vector<U32>& indices) noexcept;

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
}
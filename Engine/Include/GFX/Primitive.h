#pragma once

namespace ZE::GFX::Primitive
{
	template<typename T>
	struct Data
	{
		std::vector<T> Vertices;
		std::vector<U32> Indices;
	};

	// Generate cube vertex data
	std::vector<Float3> MakeCubeVertex() noexcept;
	// Generate cube index data
	std::vector<U32> MakeCubeIndex() noexcept;
	// Generate cube index data (front face inside cube)
	std::vector<U32> MakeCubeIndexInverted() noexcept;

	// Generate cone vertex and index data
	Data<Float3> MakeCone(U32 density) noexcept;

	// Generate Ico sphere vertex and index data
	Data<Float3> MakeSphereIco(U32 density) noexcept;
}
#pragma once

namespace ZE::GFX::Primitive
{
	// Generate cube vertex data
	std::vector<Float3> MakeCubeVertex() noexcept;
	// Generate cube index data
	std::vector<U32> MakeCubeIndex() noexcept;
	// Generate cube index data (front face inside cube)
	std::vector<U32> MakeCubeIndexInverted() noexcept;

	// Generate Ico sphere vertex and index data
	std::pair<std::vector<Float3>, std::vector<U32>> MakeSphereIco(U32 density) noexcept;
}
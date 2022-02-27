#pragma once

namespace ZE::Data
{
	// Component describing directional light params
	struct DirectionalLight
	{
		ColorF3 Color;
		float Intensity;
		ColorF3 Shadow;
	};
}
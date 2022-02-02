#pragma once

namespace ZE::Data
{
	// Component describing directional light params
	struct DirectionalLight
	{
		Float3 Direction;
		float Intensity;
		ColorF3 Color;
		float _Padding;
		ColorF3 Shadow;
	};
}
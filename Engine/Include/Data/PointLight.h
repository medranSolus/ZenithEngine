#pragma once

namespace ZE::Data
{
	// Component describing point light params
	struct PointLight
	{
		Float3 Position;
		float Intensity;
		ColorF3 Color;
		float LinearAttenuation;
		ColorF3 Shadow;
		float QuadAttenuation;
	};
}
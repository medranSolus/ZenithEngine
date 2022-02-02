#pragma once

namespace ZE::Data
{
	// Component describing spot light params
	struct SpotLight
	{
		Float3 Position;
		float Intensity;
		ColorF3 Color;
		float LinearAttenuation;
		ColorF3 Shadow;
		float QuadAttenuation;
		Float3 Direction;
		float InnerAngle;
		float OuterAngle;
	};
}
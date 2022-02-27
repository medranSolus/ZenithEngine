#pragma once

namespace ZE::Data
{
	// Component describing spot light params
	struct SpotLight
	{
		ColorF3 Color;
		float Intensity;
		ColorF3 Shadow;
		float AttnLinear;
		Float3 Direction;
		float AttnQuad;
		float InnerAngle;
		float OuterAngle;
	};
}
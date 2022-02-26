#pragma once

namespace ZE::Data
{
	// Component containing point light parameters
	struct PointLight
	{
		ColorF3 Color;
		float Intensity;
		ColorF3 Shadow;
		float AttnLinear;
		float AttnQuad;
	};
}
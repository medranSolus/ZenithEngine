#pragma once

namespace ZE::GFX
{
	// Parameters of the current display adapter
	struct DisplayProperties
	{
		Float2 RedPrimary;
		Float2 GreenPrimary;
		Float2 BluePrimary;
		Float2 WhitePoint;
		float MinLuminance;
		float MaxLuminance;
	};
}
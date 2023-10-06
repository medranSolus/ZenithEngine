#pragma once

namespace ZE::GFX
{
	// Recognized shader model used to compile shaders with
	enum class ShaderModel : U8
	{
		V5_0, V5_1,
		V6_0, V6_1, V6_2, V6_3, V6_4, V6_5, V6_6, V6_7, V6_8
	};
}
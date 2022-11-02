#pragma once

namespace ZE::GFX::Resource
{
	// Parameters associated with each vertex
	enum class InputParam : U8
	{
		// 2 component float position
		Pos2D,
		// 3 component float position
		Pos3D,
		// 2 component float UV coordinate
		TexCoord,
		// 3 component float normal
		Normal,
		// 3 component float tangent
		Tangent,
		// 4 component float tangent with handness data in .w component
		TangentPacked,
		// 3 component float bitangent
		Bitangent,
		// 4 component U8 color
		Pixel,
		// 3 component float color
		ColorF3,
		// 4 component float color
		ColorF4
	};

	// Returns shader semantic name for given parameter
	constexpr const char* GetInputSemantic(InputParam param) noexcept;
	// Returns associated format of input parameter
	constexpr PixelFormat GetInputFormat(InputParam param) noexcept;

#pragma region Functions
	constexpr const char* GetInputSemantic(InputParam param) noexcept
	{
		switch (param)
		{
		case InputParam::Pos2D:
		case InputParam::Pos3D:
			return "POSITION";
		case InputParam::TexCoord:
			return "TEXCOORD";
		case InputParam::Normal:
			return "NORMAL";
		case InputParam::Tangent:
			return "TANGENT";
		case InputParam::TangentPacked:
			return "TANGENTPACK";
		case InputParam::Bitangent:
			return "BITANGENT";
		case InputParam::Pixel:
		case InputParam::ColorF3:
		case InputParam::ColorF4:
			return "COLOR";
		}
		ZE_FAIL("Incorrect input parameter!");
		return "?";
	}

	constexpr PixelFormat GetInputFormat(InputParam param) noexcept
	{
		switch (param)
		{
		case InputParam::Pos2D:
		case InputParam::TexCoord:
			return PixelFormat::R32G32_Float;
		case InputParam::Pos3D:
		case InputParam::Normal:
		case InputParam::Tangent:
		case InputParam::Bitangent:
		case InputParam::ColorF3:
			return PixelFormat::R32G32B32_Float;
		case InputParam::Pixel:
			return PixelFormat::R32G32B32A32_UInt;
		case InputParam::TangentPacked:
		case InputParam::ColorF4:
			return PixelFormat::R32G32B32A32_Float;
		}
		ZE_FAIL("Incorrect input parameter!");
		return PixelFormat::Unknown;
	}
#pragma endregion
}
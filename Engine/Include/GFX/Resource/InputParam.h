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
		// X channel of the half float position
		PosHalf_X,
		// Y channel of the half float position
		PosHalf_Y,
		// Z channel of the half float position
		PosHalf_Z,
		// X channel of the normalized Bounding Box Quantization position
		PosBBQ_X,
		// Y channel of the normalized Bounding Box Quantization position
		PosBBQ_Y,
		// Z channel of the normalized Bounding Box Quantization position
		PosBBQ_Z,
		// 2 component float UV coordinate
		TexCoord,
		// 2 component half float UV coordinate
		TexCoordHalf,
		// 2 component UNorm16 UV coordinate
		TexCoordNorm,
		// 3 component float normal
		Normal,
		// 3 component float tangent
		Tangent,
		// 4 component float tangent with handness data in .w component
		TangentPacked,
		// 3 component float bitangent
		Bitangent,
		// Normal and tangent stored with dual octahedral encoding
		NormalTangentDualOct,
		// Bitangent handedness in from various normal-tangent encodings
		BitangentHand,
		// Normal packed with octahedral encoding and tangent stored as angle
		NormalOctTangentAngle,
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
		case InputParam::PosHalf_X:
		case InputParam::PosBBQ_X:
			return "POS_X";
		case InputParam::PosHalf_Y:
		case InputParam::PosBBQ_Y:
			return "POS_Y";
		case InputParam::PosHalf_Z:
		case InputParam::PosBBQ_Z:
			return "POS_Z";
		case InputParam::TexCoord:
		case InputParam::TexCoordHalf:
		case InputParam::TexCoordNorm:
			return "TEXCOORD";
		case InputParam::Normal:
			return "NORMAL";
		case InputParam::Tangent:
			return "TANGENT";
		case InputParam::TangentPacked:
			return "TANGENTPACK";
		case InputParam::Bitangent:
			return "BITANGENT";
		case InputParam::NormalTangentDualOct:
			return "NORM_OCT";
		case InputParam::BitangentHand:
			return "BITAN_HAND";
		case InputParam::NormalOctTangentAngle:
			return "NORM_OCT_ANGL";
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
		case InputParam::PosBBQ_X:
		case InputParam::PosBBQ_Y:
		case InputParam::PosBBQ_Z:
		case InputParam::BitangentHand:
			return PixelFormat::R16_SNorm;
		case InputParam::PosHalf_X:
		case InputParam::PosHalf_Y:
		case InputParam::PosHalf_Z:
			return PixelFormat::R16_Float;
		case InputParam::TexCoordHalf:
			return PixelFormat::R16G16_Float;
		case InputParam::TexCoordNorm:
			return PixelFormat::R16G16_UNorm;
		case InputParam::NormalOctTangentAngle:
			return PixelFormat::R16G16_SNorm;
		case InputParam::Pos2D:
		case InputParam::TexCoord:
			return PixelFormat::R32G32_Float;
		case InputParam::NormalTangentDualOct:
			return PixelFormat::R8G8B8A8_SNorm;
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
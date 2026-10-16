#pragma once
#include "GFX/Resource/InputParam.h"

namespace ZE::GFX::Vertex
{
	// What vertex packing have been applied
	typedef U8 Packing;
	// What kind of packing to apply to vertices
	enum class PackMethod : Packing
	{
		// No packing at all, using directly layout GFX::Vertex
		None = 0,

		// Position is packed into half floats
		Position_Half = 0x01,
		// Position is packed with Bounding Box Quantization
		Position_BBQ = 0x02,

		// Normal and tangent are packed using dual octahedran encoding
		NormalTangent_DualOct = 0x04,
		// Normal and tangent are packed as octahedral normal and tangent angle
		NormalTangent_OctAngle = 0x08,
		// Normal and tangent are packed as octahedral normal and tangent angle
		NormalTangent_OctDiamond = 0x10,

		// UV coords are packed as UNorm16
		UV_UNorm = 0x20,
		// UV coords are packed as half floats
		UV_Half = 0x40,

		// Pack position  separately from other vertex attributes
		SplitPositionStream = 0x80,

		// Default vertex packing
		Default = Position_BBQ | UV_Half | NormalTangent_OctDiamond | SplitPositionStream,

		// Mask for extracting position related flags
		PositionMask = Position_Half | Position_BBQ,
		// Mask to indicate which flags contribute to physical permutations of shader
		PermutationMask = PositionMask | NormalTangent_DualOct | NormalTangent_OctAngle | NormalTangent_OctDiamond
	};
	ZE_ENUM_OPERATORS(PackMethod, Packing);

#pragma pack(push, 1)
	// Current vertex layout used by the engine for CPU-side computations
	struct Raw
	{
		Float3 Position = {};
		Float2 UV = {};
		Float3 Normal = {};
		Float4 Tangent = {};
	};
#pragma pack(pop)

	// Get size of single vertex given it's packing strategy
	constexpr U8 GetVertexSize(Packing pack, bool positionOnly) noexcept;
	// Get input layout of the vertex
	constexpr std::array<std::vector<Resource::InputParam>, 2> GetLayout(Packing pack, bool positionOnly) noexcept;

	// Fill-out memory with correct vertex layout packing
	void PackData(U8* memory, Packing pack, const Float3& pos, const Float2& uv, const Float3& normal,
		const Float3& tangent, const Float3& bitangent, Vector* boxCenter, Vector* boxHalfSize) noexcept;
	// Fill-out memory with position-only stream
	void PackPositionData(U8* memory, Packing pack, const Float3& pos, const Float2& uv, const Float3& normal,
		const Float3& tangent, const Float3& bitangent, Vector* boxCenter, Vector* boxHalfSize) noexcept;
	// Fill-out memory with rest of the vertex attributes
	void PackAttributesData(U8* memory, Packing pack, const Float2& uv, const Float3& normal, const Float3& tangent, const Float3& bitangent) noexcept;
	// Get proper general suffix for shaders using model inputs
	std::string DecodeShaderSuffix(Packing pack, bool positionOnly) noexcept;

#pragma region Functions
	constexpr U8 GetVertexSize(Packing pack, bool positionOnly) noexcept
	{
		if (pack == PackMethod::Default)
			return positionOnly ? 4 * sizeof(U16) : 8 * sizeof(U16);
		if (pack == PackMethod::None)
			return sizeof(Raw);

		U8 size = 0;
		// Half and BBQ use same bit size (next member must align to 4 bytes)
		if (pack & (PackMethod::Position_Half | PackMethod::Position_BBQ))
			size += 4 * Utils::GetFormatSize(Resource::GetInputFormat(Resource::InputParam::PosHalf_X));
		else
			size += Utils::GetFormatSize(Resource::GetInputFormat(Resource::InputParam::Pos3D));

		if (positionOnly)
			return size;

		// Same bit size
		if (pack & (PackMethod::UV_UNorm | PackMethod::UV_Half))
			size += Utils::GetFormatSize(Resource::GetInputFormat(Resource::InputParam::TexCoordHalf));
		else
			size += Utils::GetFormatSize(Resource::GetInputFormat(Resource::InputParam::TexCoord));

		if (pack & PackMethod::NormalTangent_DualOct)
		{
			size += Utils::GetFormatSize(Resource::GetInputFormat(Resource::InputParam::NormalTangentDualOct));
			if ((pack & (PackMethod::Position_Half | PackMethod::Position_BBQ)) == 0)
				size += Utils::GetFormatSize(Resource::GetInputFormat(Resource::InputParam::BitangentHand));
		}
		else if (pack & (PackMethod::NormalTangent_OctAngle | PackMethod::NormalTangent_OctDiamond))
		{
			// Angle + diamond have same packing, just different codec
			size += Utils::GetFormatSize(Resource::GetInputFormat(Resource::InputParam::NormalOctTangentAngle));
			if ((pack & (PackMethod::Position_Half | PackMethod::Position_BBQ)) == 0)
				size += Utils::GetFormatSize(Resource::GetInputFormat(Resource::InputParam::BitangentHand));
		}
		else
			size += Utils::GetFormatSize(Resource::GetInputFormat(Resource::InputParam::Normal)) + Utils::GetFormatSize(Resource::GetInputFormat(Resource::InputParam::TangentPacked));
		return Math::AlignUp<U8>(size, 4U);
	}

	constexpr std::array<std::vector<Resource::InputParam>, 2> GetLayout(Packing pack, bool positionOnly) noexcept
	{
		if (pack == PackMethod::Default)
		{
			if (positionOnly)
				return { std::vector<Resource::InputParam>{ Resource::InputParam::PosBBQ_X, Resource::InputParam::PosBBQ_Y, Resource::InputParam::PosBBQ_Z, Resource::InputParam::BitangentHand } };
			return
			{
				std::vector<Resource::InputParam>{ Resource::InputParam::PosBBQ_X, Resource::InputParam::PosBBQ_Y, Resource::InputParam::PosBBQ_Z, Resource::InputParam::BitangentHand },
				std::vector<Resource::InputParam>{ Resource::InputParam::TexCoordHalf, Resource::InputParam::NormalOctTangentAngle }
			};
		}
		if (pack == PackMethod::None)
		{
			if (positionOnly)
				return { std::vector<Resource::InputParam>{ Resource::InputParam::Pos3D } };
			return
			{
				std::vector<Resource::InputParam>
				{
					Resource::InputParam::Pos3D,
					Resource::InputParam::TexCoord, Resource::InputParam::Normal, Resource::InputParam::TangentPacked
				}
			};
		}

		std::array<std::vector<Resource::InputParam>, 2> layout;
		if (pack & PackMethod::Position_BBQ)
		{
			layout.front().emplace_back(Resource::InputParam::PosBBQ_X);
			layout.front().emplace_back(Resource::InputParam::PosBBQ_Y);
			layout.front().emplace_back(Resource::InputParam::PosBBQ_Z);
			if (pack & (PackMethod::NormalTangent_DualOct | PackMethod::NormalTangent_OctAngle | PackMethod::NormalTangent_OctDiamond))
				layout.front().emplace_back(Resource::InputParam::BitangentHand);
		}
		else if (pack & PackMethod::Position_Half)
		{
			layout.front().emplace_back(Resource::InputParam::PosHalf_X);
			layout.front().emplace_back(Resource::InputParam::PosHalf_Y);
			layout.front().emplace_back(Resource::InputParam::PosHalf_Z);
			if (pack & (PackMethod::NormalTangent_DualOct | PackMethod::NormalTangent_OctAngle | PackMethod::NormalTangent_OctDiamond))
				layout.front().emplace_back(Resource::InputParam::BitangentHand);
		}
		else
			layout.front().emplace_back(Resource::InputParam::Pos3D);

		if (positionOnly)
			return layout;

		auto& attributesStream = layout.at((pack & PackMethod::SplitPositionStream) != 0);
		if (pack & PackMethod::UV_Half)
			attributesStream.emplace_back(Resource::InputParam::TexCoordHalf);
		else if (pack & PackMethod::UV_UNorm)
			attributesStream.emplace_back(Resource::InputParam::TexCoordNorm);
		else
			attributesStream.emplace_back(Resource::InputParam::TexCoord);

		if (pack & PackMethod::NormalTangent_DualOct)
		{
			attributesStream.emplace_back(Resource::InputParam::NormalTangentDualOct);
			if ((pack & (PackMethod::Position_Half | PackMethod::Position_BBQ)) == 0)
				attributesStream.emplace_back(Resource::InputParam::BitangentHand);
		}
		else if (pack & (PackMethod::NormalTangent_OctAngle | PackMethod::NormalTangent_OctDiamond))
		{
			attributesStream.emplace_back(Resource::InputParam::NormalOctTangentAngle);
			if ((pack & (PackMethod::Position_Half | PackMethod::Position_BBQ)) == 0)
				attributesStream.emplace_back(Resource::InputParam::BitangentHand);
		}
		else
		{
			attributesStream.emplace_back(Resource::InputParam::Normal);
			attributesStream.emplace_back(Resource::InputParam::TangentPacked);
		}
		return layout;
	}
#pragma endregion
}
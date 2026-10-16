#include "GFX/Vertex.h"

namespace ZE::GFX::Vertex
{
	static bool IsBitangentNegative(const Float3& normal, const Float3& tangent, const Float3& bitangent) noexcept
	{
		// Get handedness of bitangent to later multiply when recomputing bitangent
		Vector tan = Math::XMLoadFloat3(&tangent);
		Vector nrm = Math::XMLoadFloat3(&normal);
		Vector bitan = Math::XMLoadFloat3(&bitangent);

		return Math::XMVectorGetX(Math::XMVector3Dot(Math::XMVector3Cross(tan, nrm), bitan)) < 0.0f;
	}

	static S16 GetTangentAngle(const Float3& normal, const Float3& tangent, const Float3& bitangent) noexcept
	{
		float angle = Math::EncodeTangentAngle(normal, tangent);
		angle = (angle + Math::PI) / Math::PI2;
		return static_cast<S16>(angle * static_cast<float>(IsBitangentNegative(normal, tangent, bitangent) ? -INT16_MAX : INT16_MAX));
	}

	static S16 GetTangentDiamond(const Float3& normal, const Float3& tangent, const Float3& bitangent) noexcept
	{
		return static_cast<S16>(Math::EncodeTangentDiamond(normal, tangent) * static_cast<float>(IsBitangentNegative(normal, tangent, bitangent) ? -INT16_MAX : INT16_MAX));
	}

	void PackData(U8* memory, Packing pack, const Float3& pos, const Float2& uv, const Float3& normal,
		const Float3& tangent, const Float3& bitangent, Vector* boxCenter, Vector* boxHalfSize) noexcept
	{
		ZE_ASSERT(memory, "Empty output vertex memory!");

		PackPositionData(memory, pack, pos, uv, normal, tangent, bitangent, boxCenter, boxHalfSize);
		PackAttributesData(memory + GetVertexSize(pack, true), pack, uv, normal, tangent, bitangent);
	}

	void PackPositionData(U8* memory, Packing pack, const Float3& pos, const Float2& uv, const Float3& normal,
		const Float3& tangent, const Float3& bitangent, Vector* boxCenter, Vector* boxHalfSize) noexcept
	{
		ZE_ASSERT(memory, "Empty output vertex memory!");

		if (pack & PackMethod::Position_BBQ)
		{
			ZE_ASSERT(boxCenter && boxHalfSize, "When using BBQ encoding bounding box center and half size values cannot be empty!");

			Vector position = Math::XMLoadFloat3(&pos);
			position = Math::XMVectorScale(Math::XMVectorDivide(Math::XMVectorSubtract(position, *boxCenter), *boxHalfSize), INT16_MAX);

			*reinterpret_cast<S16*>(memory) = static_cast<S16>(Math::XMVectorGetX(position));
			memory += sizeof(S16);
			*reinterpret_cast<S16*>(memory) = static_cast<S16>(Math::XMVectorGetY(position));
			memory += sizeof(S16);
			*reinterpret_cast<S16*>(memory) = static_cast<S16>(Math::XMVectorGetZ(position));

			if (pack & PackMethod::NormalTangent_OctDiamond)
			{
				memory += sizeof(S16);
				*reinterpret_cast<S16*>(memory) = GetTangentDiamond(normal, tangent, bitangent);
				memory += sizeof(S16);
			}
			else if (pack & PackMethod::NormalTangent_DualOct)
			{
				memory += sizeof(S16);
				*reinterpret_cast<S16*>(memory) = IsBitangentNegative(normal, tangent, bitangent) ? -INT16_MAX : INT16_MAX;
				memory += sizeof(S16);
			}
			else if (pack & PackMethod::NormalTangent_OctAngle)
			{
				memory += sizeof(S16);
				*reinterpret_cast<S16*>(memory) = GetTangentAngle(normal, tangent, bitangent);
				memory += sizeof(S16);
			}
			else
				memory += 2 * sizeof(S16);
		}
		else if (pack & PackMethod::Position_Half)
		{
			*reinterpret_cast<U16*>(memory) = Math::FP16::EncodeFloat16(pos.x);
			memory += sizeof(U16);
			*reinterpret_cast<U16*>(memory) = Math::FP16::EncodeFloat16(pos.y);
			memory += sizeof(U16);
			*reinterpret_cast<U16*>(memory) = Math::FP16::EncodeFloat16(pos.z);

			if (pack & PackMethod::NormalTangent_OctDiamond)
			{
				memory += sizeof(U16);
				*reinterpret_cast<S16*>(memory) = GetTangentDiamond(normal, tangent, bitangent);
				memory += sizeof(U16);
			}
			else if (pack & PackMethod::NormalTangent_DualOct)
			{
				memory += sizeof(U16);
				*reinterpret_cast<S16*>(memory) = IsBitangentNegative(normal, tangent, bitangent) ? -INT16_MAX : INT16_MAX;
				memory += sizeof(U16);
			}
			else if (pack & PackMethod::NormalTangent_OctAngle)
			{
				memory += sizeof(U16);
				*reinterpret_cast<S16*>(memory) = GetTangentAngle(normal, tangent, bitangent);
				memory += sizeof(U16);
			}
			else
				memory += 2 * sizeof(U16);
		}
		else
		{
			*reinterpret_cast<Float3*>(memory) = pos;
			memory += sizeof(Float3);
		}

	}

	void PackAttributesData(U8* memory, Packing pack, const Float2& uv, const Float3& normal, const Float3& tangent, const Float3& bitangent) noexcept
	{
		ZE_ASSERT(memory, "Empty output vertex memory!");

		if (pack & PackMethod::UV_Half)
		{
			*reinterpret_cast<U16*>(memory) = Math::FP16::EncodeFloat16(uv.x);
			memory += sizeof(U16);
			*reinterpret_cast<U16*>(memory) = Math::FP16::EncodeFloat16(uv.y);
			memory += sizeof(U16);
		}
		else if (pack & PackMethod::UV_UNorm)
		{
			*reinterpret_cast<U16*>(memory) = static_cast<U16>(uv.x * UINT16_MAX);
			memory += sizeof(U16);
			*reinterpret_cast<U16*>(memory) = static_cast<U16>(uv.y * UINT16_MAX);
			memory += sizeof(U16);
		}
		else
		{
			*reinterpret_cast<Float2*>(memory) = uv;
			memory += sizeof(Float2);
		}

		if (pack & (PackMethod::NormalTangent_OctAngle | PackMethod::NormalTangent_OctDiamond))
		{
			Float2 nrm = Math::EncodeOctahedral(normal);
			*reinterpret_cast<S16*>(memory) = static_cast<S16>(nrm.x * INT16_MAX);
			memory += sizeof(S16);
			*reinterpret_cast<S16*>(memory) = static_cast<S16>(nrm.y * INT16_MAX);

			// If can't pack into position free place then add separate member
			if ((pack & (PackMethod::Position_Half | PackMethod::Position_BBQ)) == 0)
				*reinterpret_cast<S16*>(memory + sizeof(S16)) = (pack & PackMethod::NormalTangent_OctAngle) ? GetTangentAngle(normal, tangent, bitangent) : GetTangentDiamond(normal, tangent, bitangent);
		}
		else if (pack & PackMethod::NormalTangent_DualOct)
		{
			Float2 nrm = Math::EncodeOctahedral(normal);
			Float2 tan = Math::EncodeOctahedral(tangent);
			Float4 oct = { nrm.x, nrm.y, tan.x, tan.y };
			Vector coded = Math::XMVectorScale(Math::XMLoadFloat4(&oct), INT8_MAX);

			*memory = static_cast<U8>(Math::XMVectorGetX(coded));
			memory += sizeof(U8);
			*memory = static_cast<U8>(Math::XMVectorGetY(coded));
			memory += sizeof(U8);
			*memory = static_cast<U8>(Math::XMVectorGetZ(coded));
			memory += sizeof(U8);
			*memory = static_cast<U8>(Math::XMVectorGetW(coded));

			// If can't pack into position free place then add separate member
			if ((pack & (PackMethod::Position_Half | PackMethod::Position_BBQ)) == 0)
				*reinterpret_cast<S16*>(memory + sizeof(U8)) = IsBitangentNegative(normal, tangent, bitangent) ? -INT16_MAX : INT16_MAX;
		}
		else
		{
			*reinterpret_cast<Float3*>(memory) = normal;
			memory += sizeof(Float3);
			*reinterpret_cast<Float3*>(memory) = tangent;
			*reinterpret_cast<float*>(memory + sizeof(Float3)) = IsBitangentNegative(normal, tangent, bitangent) ? -1.0f : 1.0f;
		}
	}

	std::string DecodeShaderSuffix(Packing pack, bool positionOnly) noexcept
	{
		if (pack == PackMethod::Default)
			return positionOnly ? "_B" : "_BD";
		if ((pack & (positionOnly ? PackMethod::PositionMask : PackMethod::PermutationMask)) == PackMethod::None)
			return "";

		std::string suffix = "_";
		if (pack & PackMethod::Position_BBQ)
			suffix += 'B';
		else if (pack & PackMethod::Position_Half)
			suffix += 'H';

		if (positionOnly)
			return suffix;

		if (pack & PackMethod::NormalTangent_DualOct)
			suffix += 'O';
		else if (pack & PackMethod::NormalTangent_OctAngle)
			suffix += 'A';
		else if (pack & PackMethod::NormalTangent_OctDiamond)
			suffix += 'D';
		return suffix;
	}
}
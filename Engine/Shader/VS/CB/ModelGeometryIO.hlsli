#ifndef MODEL_GEOMETRY_IO_VS_HLSLI
#define MODEL_GEOMETRY_IO_VS_HLSLI
#include "Buffers.hlsli"
#include "Utils.hlsli"

struct PackedPos
{
#ifdef _ZE_PACKED_POSITION
	float PosX : POS_X;
	float PosY : POS_Y;
	float PosZ : POS_Z;
#else
	float3 Pos : POSITION;
#endif
};

struct ModelVSIn
{
	PackedPos PosPack;
#if defined(_ZE_PACKED_POSITION) && (defined(_ZE_NRM_TAN_DUAL_OCT) || defined(_ZE_NRM_OCT_TAN_ANGL) || defined(_ZE_NRM_OCT_TAN_DIAMOND))
	float BitanHand : BITAN_HAND;
#endif
	float2 UV : TEXCOORD;
#ifdef _ZE_NRM_TAN_DUAL_OCT
	float4 DualOct : NORM_OCT;
#	ifndef _ZE_PACKED_POSITION
	float BitanHand : BITAN_HAND;
#	endif
#elif defined(_ZE_NRM_OCT_TAN_ANGL) || defined(_ZE_NRM_OCT_TAN_DIAMOND)
	float2 NormalOct : NORM_OCT_ANGL;
#	ifndef _ZE_PACKED_POSITION
	float BitanHand : BITAN_HAND;
#	endif
#else
	float3 Normal : NORMAL;
	float4 Tangent : TANGENTPACK;
#endif
};

#ifdef _ZE_BBQ_POS_PACK
struct DecodeBBQ
{
	float3 BoxCenter;
	float3 BoxHalfSize;
};

// To correctly use this cbuffer, define 'ZE_BBQ_CB_RANGE' to indicate binding range used for constant buffer
CBUFFER(bbqDecode, DecodeBBQ, 1, ZE_BBQ_CB_RANGE);
#endif

float3 GetPosition(const in PackedPos pospack)
{
#ifdef _ZE_PACKED_POSITION
	float3 pos = float3(pospack.PosX, pospack.PosY, pospack.PosZ);
#else
	float3 pos = pospack.Pos;
#endif
	
#ifdef _ZE_BBQ_POS_PACK
	pos = pos * cb_bbqDecode.BoxHalfSize + cb_bbqDecode.BoxCenter;
#endif
	return pos;
}

float3 GetNormal(const in ModelVSIn vsi)
{
#ifdef _ZE_NRM_TAN_DUAL_OCT
	return DecodeOctahedral(vsi.DualOct.xy);
#elif defined(_ZE_NRM_OCT_TAN_ANGL) || defined(_ZE_NRM_OCT_TAN_DIAMOND)
	return DecodeOctahedral(vsi.NormalOct);
#else
	return vsi.Normal;
#endif
}

#if defined(_ZE_NRM_OCT_TAN_ANGL) || defined(_ZE_NRM_OCT_TAN_DIAMOND)
#define GetTangent(vsi, nrm) _GetTangent(vsi, nrm)

float3 _GetTangent(const in ModelVSIn vsi, const in float3 normal)
#else
#define GetTangent(vsi, nrm) _GetTangent(vsi)

float3 _GetTangent(const in ModelVSIn vsi)
#endif
{
#ifdef _ZE_NRM_TAN_DUAL_OCT
	return DecodeOctahedral(vsi.DualOct.zw);
#elif defined(_ZE_NRM_OCT_TAN_ANGL)
	return DecodeTangentAngle(normal, (abs(vsi.BitanHand) * 2.0f - 1.0f) * PI);
#elif defined(_ZE_NRM_OCT_TAN_DIAMOND)
	return DecodeTangentDiamond(normal, abs(vsi.BitanHand));
#else
	return vsi.Tangent.xyz;
#endif
}

float GetBitanHandedness(const in ModelVSIn vsi)
{
#ifdef _ZE_NRM_TAN_DUAL_OCT
	return vsi.BitanHand;
#elif defined(_ZE_NRM_OCT_TAN_ANGL) || defined(_ZE_NRM_OCT_TAN_DIAMOND)
	return SignNotZero(vsi.BitanHand);
#else
	return vsi.Tangent.w;
#endif
}

#endif // MODEL_GEOMETRY_IO_VS_HLSLI
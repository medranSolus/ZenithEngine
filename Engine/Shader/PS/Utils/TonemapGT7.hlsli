#ifndef TONEMAP_GT7_UTILS_HLSLI
#define TONEMAP_GT7_UTILS_HLSLI
#include "Utils.hlsli"
// Source from SigGraph 2025
// Driving Toward Reality: Physically Based Tone Mapping and Perceptual Fidelity in Gran Turismo 7
// https://blog.selfshadow.com/publications/s2025-shading-course/#course_content

// Conversion to Unified Color Space
#ifdef _GT7_UCS_JZAZBZ
// Jzazbz conversion
// Ref: Muhammad Safdar, Guihua Cui, Youn Jin Kim, and Ming Ronnier Luo
// "Perceptually uniform color space for image signals including high dynamic
// range and wide gamut," Opt. Express 25, 15131-15151 (2017)
// Note: Coefficients adjusted for linear Rec.2020

static const float JZAZBZ_EXPONENT_SCALE = 1.7f;

float3 ApplyUCS(const in float3 rgb)
{
	static const float3x3 JZAZBZ_INPUT_MAT =
	{
		{ 0.530004f, 0.355704f, 0.086090f },
		{ 0.289388f, 0.525395f, 0.157481f },
		{ 0.091098f, 0.147588f, 0.734234f }
	};
	static const float2x3 JZAZBZ_INTER_MAT =
	{
		{ 3.524000f, -4.066708f, 0.542708f },
		{ 0.199076f, 1.096799f, -1.295875f }
	};

	const float3 lms = ApplyPQ(mul(JZAZBZ_INPUT_MAT, rgb), JZAZBZ_EXPONENT_SCALE);
    const float iz = dot(lms.rg, 0.5f);
	
	const float jr = (0.44f * iz) / (1.0f - 0.56f * iz) - 1.6295499532821566e-11f;
	const float2 jgb = mul(JZAZBZ_INTER_MAT, lms);
	return float3(jr, jgb);
}

float3 DeleteUCS(const in float3 ucs)
{
	static const float3x2 JZAZBZ_INTER_MAT =
	{
		{ 1.386050432715393e-1f, 5.804731615611869e-2f },
		{ -1.386050432715393e-1f, -5.804731615611869e-2f },
		{ -9.601924202631895e-2f, -8.118918960560390e-1f }
	};
	static const float3x3 JZAZBZ_OUTPUT_MAT =
	{
		{ 2.990669f, -2.049742f, 0.086090f },
		{ -1.634525f, 3.145627f, -0.483037f },
		{ -0.042505f, -0.377983f, 1.448019f }
	};

	const float jz = ucs.r + 1.6295499532821566e-11f;
	const float iz = jz / (0.44f + 0.56f * jz);
	
	const float3 lms = DeletePQ(mul(JZAZBZ_INTER_MAT, ucs.gb) + iz, JZAZBZ_EXPONENT_SCALE);
	return mul(JZAZBZ_OUTPUT_MAT, lms);
}
#else
// ICtCp conversion.
// Reference: ITU-T T.302 https://www.itu.int/rec/T-REC-T.302/en

float3 ApplyUCS(const in float3 rgb)
{
	static const float3x3 ICTCP_INPUT_MAT =
	{
		{ 0.412109375f, 0.52392578125f, 0.06396484375f },
		{ 0.166748046875f, 0.720458984375f, 0.11279296875f },
		{ 0.024169921875f, 0.075439453125f, 0.900390625f }
	};
	static const float3x3 ICTCP_INTER_MAT =
	{
		{ 0.5f, 0.5f, 0.0f },
		{ 1.61376953125f, -3.323486328125f, 1.709716796875f },
		{ 4.378173828125f, -4.24560546875f, -0.132568359375f }
	};
	
	const float3 lms = ApplyPQ(mul(ICTCP_INPUT_MAT, rgb), 1.0f);
	return mul(ICTCP_INTER_MAT, lms);
}

float3 DeleteUCS(const in float3 ucs)
{
	static const float3x3 ICTCP_INTER_MAT =
	{
		{ 1.0f, 0.00860904f, 0.11103f },
		{ 1.0f, -0.00860904f, -0.11103f },
		{ 1.0f, 0.560031f, -0.320627f }
	};
	static const float3x3 ICTCP_OUTPUT_MAT =
	{
		{ 3.43661f, -2.50645f, 0.0698454f },
		{ -0.79133f, 1.9836f, -0.192271f },
		{ -0.0259499f, -0.0989137f, 1.12486f }
	};
	
	const float3 lms = DeletePQ(mul(ICTCP_INTER_MAT, ucs), 1.0f);
	return max(mul(ICTCP_OUTPUT_MAT, lms), 0.0f);
}
#endif

float ApplyChromaCurve(const in float start, const in float end, const in float x)
{
	return 1.0f - smoothstep(start, end, x);
}

float3 ApplyGt7Curve(const in float3 color,
	const in float paramA, const in float paramB, const in float paramC,
	const in float midPoint, const in float toeStrength, const in float shoulderTreshold)
{
	const float3 x = max(color, 0.0f);
	
    // Shoulder mapping for highlights
	const float3 shoulder = paramA + paramB * exp(x * paramC);
	
	const float3 toeMapped = midPoint * pow(x / midPoint, toeStrength);
	const float3 linearWeight = smoothstep(0.0f, midPoint, x);
	
	return lerp(lerp(toeMapped, x, linearWeight), shoulder, step(shoulderTreshold, x));
}

// Input:  linear Rec.2020 RGB
// Output: tone-mapped RGB
//         - in SDR mode: mapped to [0, 1], ready for sRGB OETF
//         - in HDR mode: mapped to [0, luminanceTarget], ready for PQ inverse-EOTF
// Note: luminanceTarget represents the display's target peak luminance converted to a frame buffer value.
//       The returned values are suitable for applying the appropriate OETF to generate final output signal
float3 GetGranTurismo7(const in float3 color, const in float exposure,
	const in float paramA, const in float paramB, const in float paramC,
	const in float midPoint, const in float toeStrength, const in float shoulderTreshold,
	const in float fadeInStart, const in float fadeInEnd,
	const in float luminanceTargetUCS, const in float luminanceTarget,
	const in float blendRatio, const in float sdrCorrection)
{
	const float3 x = color * exposure;
	// Convert to UCS to separate luminance and chroma
	const float3 ucs = ApplyUCS(x);
	const float3 skewedColor = ApplyGt7Curve(x, paramA, paramB, paramC, midPoint, toeStrength, shoulderTreshold);
	
	const float chromaScale = ApplyChromaCurve(fadeInStart, fadeInEnd, ucs.r / luminanceTargetUCS);
	const float3 scaledColor = DeleteUCS(float3(ApplyUCS(skewedColor).r, ucs.gb * chromaScale));
	
    // When using SDR, apply the correction factor, for HDR it should be just 1
	return min(lerp(skewedColor, scaledColor, blendRatio), luminanceTarget) * sdrCorrection;
}

#endif // TONEMAP_GT7_UTILS_HLSLI
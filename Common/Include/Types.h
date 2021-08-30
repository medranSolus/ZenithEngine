#pragma once
#define _USE_MATH_DEFINES
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cfloat>
#include <cassert>
#include <intrin.h>
#pragma intrinsic(__rdtsc, __faststorefence)

namespace ZE::External
{
#include "DirectXMath.h"
#include "DirectXCollision.h"
}
namespace ZE::Math
{
	using namespace External::DirectX;
}

#pragma region Base types
	typedef uint8_t U8;
	typedef uint16_t U16;
	typedef uint32_t U32;
	typedef uint64_t U64;

	typedef int8_t S8;
	typedef int16_t S16;
	typedef int32_t S32;
	typedef int64_t S64;
#pragma endregion

#pragma region Atomic types
	typedef std::atomic_uint8_t UA8;
	typedef std::atomic_uint16_t UA16;
	typedef std::atomic_uint32_t UA32;
	typedef std::atomic_uint64_t UA64;

	typedef std::atomic_int8_t SA8;
	typedef std::atomic_int16_t SA16;
	typedef std::atomic_int32_t SA32;
	typedef std::atomic_int64_t SA64;
#pragma endregion

#pragma region Vector types
	typedef ZE::External::DirectX::XMFLOAT2 Float2;
	typedef ZE::External::DirectX::XMFLOAT3 Float3;
	typedef ZE::External::DirectX::XMFLOAT4 Float4;
	// SSE 4D Vector
	typedef ZE::External::DirectX::XMVECTOR Vector;
	struct UInt2 { U32 x, y; };
	struct UInt3 { U32 x, y, z; };
	struct UInt4 { U32 x, y, z, w; };
	struct SInt2 { S32 x, y; };
	struct SInt3 { S32 x, y, z; };
	struct SInt4 { S32 x, y, z, w; };
#pragma endregion

#pragma region Matrix types
	typedef ZE::External::DirectX::XMFLOAT3X3 Float3x3;
	typedef ZE::External::DirectX::XMFLOAT4X4 Float4x4;
	// SSE 4x4 Matrix
	typedef ZE::External::DirectX::XMMATRIX Matrix;
#pragma endregion
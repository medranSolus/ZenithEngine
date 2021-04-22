#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cfloat>
#include <cassert>
#include <intrin.h>
#pragma intrinsic(__rdtsc, __faststorefence)

namespace External
{
	#include "DirectXMath.h"
	#include "DirectXCollision.h"
}
namespace Math
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

#pragma region Vector types
	typedef External::DirectX::XMFLOAT2 Float2;
	typedef External::DirectX::XMFLOAT3 Float3;
	typedef External::DirectX::XMFLOAT4 Float4;
	// SSE 4D Vector
	typedef External::DirectX::XMVECTOR Vector;
#pragma endregion

#pragma region Matrix types
	typedef External::DirectX::XMFLOAT3X3 Float3x3;
	typedef External::DirectX::XMFLOAT4X4 Float4x4;
	// SSE 4x4 Matrix
	typedef External::DirectX::XMMATRIX Matrix;
#pragma endregion
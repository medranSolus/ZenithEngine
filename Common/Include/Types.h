#pragma once
#define _USE_MATH_DEFINES
#include "Intrinsics.h"
#include <cmath>
#include <cstddef>
#include <cfloat>

namespace ZE::External
{
#include "WarningGuardOn.h"
#include "DirectXMath.h"
#include "DirectXCollision.h"
#include "WarningGuardOff.h"
}
namespace ZE::Math
{
	using namespace External::DirectX;
}

#pragma region Vector types
	typedef ZE::External::DirectX::XMFLOAT2 Float2;
	typedef ZE::External::DirectX::XMFLOAT3 Float3;
	typedef ZE::External::DirectX::XMFLOAT4 Float4;
	// SSE 4D Vector
	typedef ZE::External::DirectX::XMVECTOR Vector;
#pragma endregion

#pragma region Matrix types
	typedef ZE::External::DirectX::XMFLOAT3X3 Float3x3;
	typedef ZE::External::DirectX::XMFLOAT4X4 Float4x4;
	// SSE 4x4 Matrix
	typedef ZE::External::DirectX::XMMATRIX Matrix;
#pragma endregion
#pragma once
#include "WarningGuardOn.h"
#include <wrl.h>
#include "WarningGuardOff.h"

namespace ZE::GFX::API::DX
{
	// Smart pointer managing COM interface counting (move to winrt::com_ptr when C++20 is enabled)
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
}
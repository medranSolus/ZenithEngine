#pragma once
#pragma warning(disable:4265)
#include <wrl.h>
#pragma warning(default:4265)

namespace ZE::GFX::API::DX
{
	// Smart pointer managing COM interface counting (move to winrt::com_ptr when C++20 is enabled)
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
}
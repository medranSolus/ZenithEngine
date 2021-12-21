#pragma once
// Headers needed for DirectX Graphics Infrastructure
#include "GFX/DX.h"
#include "DebugInfoManager.h"
#include <dxgi1_6.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>

namespace ZE::GFX::API::DX
{
	// Creates DXGI Factory
	ComPtr<IDXGIFactory7> CreateFactory(
#ifdef _ZE_MODE_DEBUG
		DebugInfoManager& debugManager
#endif
	);

	// Creates DXGI Factory and enumerates available GPU adapters in order of highest performant
	ComPtr<IDXGIAdapter4> CreateAdapter(
#ifdef _ZE_MODE_DEBUG
		DebugInfoManager& debugManager
#endif
	);

	// Creates swap chain for window and returns present flags
	UINT CreateSwapChain(ComPtr<IDXGIFactory7> factory, IUnknown* device, HWND window, ComPtr<IDXGISwapChain4>& swapChain
#ifdef _ZE_MODE_DEBUG
		, DebugInfoManager& debugManager
#endif
	);
}
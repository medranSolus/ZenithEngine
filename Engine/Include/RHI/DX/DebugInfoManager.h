#pragma once
#include "Platform/WinAPI/ComPtr.h"
ZE_WARNING_PUSH
#include <dxgidebug.h>
ZE_WARNING_POP

namespace ZE::RHI::DX
{
	// Enable ComPtr for all DX namespace
	using Platform::WinAPI::ComPtr;

	// Wrappers for DXGI interfaces (rest is in DXGI.h)
	typedef IDXGIInfoQueue IInfoQueue;
	typedef IDXGIDebug1    IDebug;

	// Retrieving info from DirectX Debug Layer
	class DebugInfoManager final
	{
		static inline DebugInfoManager* instance = nullptr;

		HMODULE dxgiDebugModule = nullptr;
		ComPtr<IInfoQueue> infoQueue = nullptr;
		ComPtr<IDebug> debug = nullptr;

	public:
		DebugInfoManager() = default;
		ZE_CLASS_MOVE(DebugInfoManager);
		~DebugInfoManager();

		// Register newly created DebugInfoManager interface to enable checking of debug layer messages
		static void Register(DebugInfoManager& mgr) noexcept { instance = &mgr; }

		static Expected<DebugInfoManager> Create() noexcept;
		static bool CheckMessages() noexcept;
	};
}
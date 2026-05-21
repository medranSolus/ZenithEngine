#pragma once
#if _ZE_FFX_API_ENABLED
#	include "GFX/External/FfxApiFunctions.h"
#	include "GFX/Device.h"

namespace ZE::RHI::DX12::External
{
	class FfxApiInterface final
	{
		HMODULE ffxApiDll = nullptr;
		PfnFfxCreateContext ffxCreateContext = nullptr;
		GFX::External::FfxApiFunctions ffxFunctions = {};

		void Destroy() noexcept;
		void MoveFrom(FfxApiInterface&& ffxInt) noexcept;

	public:
		FfxApiInterface() = default;
		ZE_CLASS_NO_COPY(FfxApiInterface);
		FfxApiInterface(FfxApiInterface&& ffxInt) noexcept { MoveFrom(std::move(ffxInt)); }
		FfxApiInterface& operator=(FfxApiInterface&& ffxInt) noexcept { Destroy(); MoveFrom(std::move(ffxInt)); return *this; }
		~FfxApiInterface() { Destroy(); }

		static Expected<FfxApiInterface> Create() noexcept;

		constexpr bool IsInitialized() const noexcept { return ffxApiDll; }
		constexpr const GFX::External::FfxApiFunctions* GetFunctions() const noexcept { return &ffxFunctions; }

		ffxReturnCode_t CreateFfxCtx(GFX::Device& dev, ffxContext* ctx, ffxCreateContextDescHeader& ctxHeader) const noexcept;
	};
}
#endif
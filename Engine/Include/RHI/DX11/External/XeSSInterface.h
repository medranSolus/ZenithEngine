#pragma once
#if _ZE_XESS_ENABLED
#	include "GFX/Pipeline/Framebuffer.h"
ZE_WARNING_PUSH
#	include "xess/xess.h"
ZE_WARNING_POP

namespace ZE::RHI::DX11::External
{
	class XeSSInterface final
	{
		xess_context_handle_t ctx = nullptr;
		bool ctxInit = false;

		void Destroy() noexcept;
		void MoveFrom(XeSSInterface&& xess) noexcept;
		Status CreateCtx(Device& dev) noexcept;

	public:
		XeSSInterface() = default;
		ZE_CLASS_NO_COPY(XeSSInterface);
		XeSSInterface(XeSSInterface&& xess) noexcept { MoveFrom(std::move(xess)); }
		XeSSInterface& operator=(XeSSInterface&& xess) noexcept { Destroy(); MoveFrom(std::move(xess)); return *this; }
		~XeSSInterface() { Destroy(); }

		static Expected<XeSSInterface> Create(GFX::Device& dev) noexcept;

		constexpr bool IsInitialized() const noexcept { return ctx != nullptr; }
		constexpr bool IsCtxInitialized() const noexcept { return ctxInit; }
		constexpr xess_context_handle_t GetCtx() const noexcept { return ctx; }
		constexpr bool IsAliasableResourcesSupported() const noexcept { return false; }
		constexpr U64 GetAliasableBufferRegionSize() const noexcept { return 0; }
		constexpr U64 GetAliasableTextureRegionSize() const noexcept { return 0; }
		constexpr void SetAliasableResources(RID buffer, RID texture) noexcept {}

		Status InitializeCtx(GFX::Device& dev, UInt2 targetRes, xess_quality_settings_t quality, U32 flags) noexcept;
		Status FreeCtx(GFX::Device& dev) noexcept;
		Status Execute(GFX::Device& dev, GFX::Pipeline::FrameBuffer& buffers, GFX::CommandList& cl,
			RID color, RID motionVectors, RID depth, RID exposure, RID responsive, RID output, const Float2& jitter, bool reset) const noexcept;
	};
}
#endif
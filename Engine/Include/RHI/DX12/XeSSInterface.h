#pragma once
#if _ZE_XESS_ENABLED
#	include "GFX/Pipeline/Framebuffer.h"
ZE_WARNING_PUSH
#	include "xess/xess.h"
ZE_WARNING_POP

namespace ZE::RHI::DX12
{
	class XeSSInterface final
	{
		xess_context_handle_t ctx = nullptr;
		DescriptorInfo descInfo = {};
		xess_2d_t outputRes = { 0, 0 };
		xess_quality_settings_t qualityMode = XESS_QUALITY_SETTING_AA;
		U32 initFlags = 0;
		U64 aliasBufferRegionSize = 0;
		U64 aliasTextureRegionSize = 0;
		RID aliasBufferRegion = INVALID_RID;
		RID aliasTextureRegion = INVALID_RID;
		Device* srcDev = nullptr;

		void Destroy(bool destroyCtx = true) noexcept;
		void MoveFrom(XeSSInterface&& xess) noexcept;

	public:
		XeSSInterface() = default;
		ZE_CLASS_NO_COPY(XeSSInterface);
		XeSSInterface(XeSSInterface&& xess) noexcept { MoveFrom(std::move(xess)); }
		XeSSInterface& operator=(XeSSInterface&& xess) noexcept { Destroy(); MoveFrom(std::move(xess)); return *this; }
		~XeSSInterface() { Destroy(); }

		static Expected<XeSSInterface> Create(GFX::Device& dev) noexcept;

		constexpr bool IsInitialized() const noexcept { return descInfo.Handle != nullptr; }
		constexpr xess_context_handle_t GetCtx() const noexcept { return ctx; }
		constexpr bool IsAliasableResourcesSupported() const noexcept { return aliasBufferRegionSize || aliasTextureRegionSize; }
		constexpr U64 GetAliasableBufferRegionSize() const noexcept { return aliasBufferRegionSize; }
		constexpr U64 GetAliasableTextureRegionSize() const noexcept { return aliasTextureRegionSize; }
		constexpr void SetAliasableResources(RID buffer, RID texture) noexcept { aliasBufferRegion = buffer; aliasTextureRegion = texture; }

		Status InitializeCtx(GFX::Device& dev, UInt2 targetRes, xess_quality_settings_t quality, U32 flags) noexcept;
		void FreeCtx(GFX::Device& dev) noexcept;
		Status Execute(GFX::Device& dev, GFX::Pipeline::FrameBuffer& buffers, GFX::CommandList& cl,
			RID color, RID motionVectors, RID depth, RID exposure, RID responsive, RID output, const Float2& jitter, bool reset) const noexcept;

		// Gfx API Internal

		constexpr RID GetAliasableBufferResource() const noexcept { return aliasBufferRegion; }
		constexpr RID GetAliasableTextureResource() const noexcept { return aliasTextureRegion; }

		Status FinishInitialization(IHeap* buffHeap, U64 buffHeapOffset, IHeap* texHeap, U64 texHeapOffset) const noexcept;
	};
}
#endif
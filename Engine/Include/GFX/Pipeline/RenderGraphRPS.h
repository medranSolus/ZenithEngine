#pragma once
#include "GFX/Graphics.h"
#include "WarningGuardOn.h"
#include "rps/rps.h"
#include "WarningGuardOff.h"

namespace ZE::GFX::Pipeline
{
	// Building and managing structure of render passes with RenderPipelineShaders
	class RenderGraphRPS
	{
		RpsDevice rpsDevice = RPS_NULL_HANDLE;
		RpsRenderGraph renderGraph = RPS_NULL_HANDLE;

		static void* RpsAllocCallback(void* pContext, size_t size, size_t alignment) noexcept;
		static void RpsFreeCallback(void* pUserContext, void* buffer) noexcept;
		static void* RpsReallocCallback(void* pUserContext, void* oldBuffer, size_t oldSize, size_t newSize, size_t alignment) noexcept;
		static void RpsPrintCallback(void* pContext, const char* format, ...) noexcept;
		static void RpsPrintVarCallback(void* pContext, const char* format, va_list vl) noexcept;
		static void RpsDeviceDestroyCallback(RpsDevice hDevice) noexcept;
		static void DefaultNodeCallback(const RpsCmdCallbackContext* pContext) noexcept;

	public:
		RenderGraphRPS(GFX::Graphics& gfx);
		ZE_CLASS_DELETE(RenderGraphRPS);
		virtual ~RenderGraphRPS();
	};
}
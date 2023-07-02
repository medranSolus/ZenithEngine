#include "GFX/Pipeline/RenderGraphRPS.h"
#include <cstdarg>

// Variable holding result of last RPS call
#define ZE_RPS_EXCEPT_RESULT __rpsResult

// Enables useage of ZE_RPS_* macros in current scope
#define ZE_RPS_ENABLE() RpsResult ZE_RPS_EXCEPT_RESULT

// Before using needs call to ZE_RPS_ENABLE()
// Checks RpsResult returned via function and throws on error
#define	ZE_RPS_THROW_FAILED(call) do { if (RPS_FAILED(ZE_RPS_EXCEPT_RESULT = (call))) { ZE_BREAK(); throw ZE_RGC_EXCEPT(rpsResultGetName(ZE_RPS_EXCEPT_RESULT)); } } while (false)

namespace ZE::GFX::Pipeline
{
	void* RenderGraphRPS::RpsAllocCallback(void* pContext, size_t size, size_t alignment) noexcept
	{
#if _ZE_COMPILER_MSVC
		return _aligned_malloc(size, alignment);
#else
		return std::aligned_alloc(alignment, size);
#endif
	}

	void RenderGraphRPS::RpsFreeCallback(void* pUserContext, void* buffer) noexcept
	{
#if _ZE_COMPILER_MSVC
		return _aligned_free(buffer);
#else
		return free(buffer);
#endif
	}

	void* RenderGraphRPS::RpsReallocCallback(void* pUserContext, void* oldBuffer, size_t oldSize, size_t newSize, size_t alignment) noexcept
	{
#if _ZE_COMPILER_MSVC
		return _aligned_realloc(oldBuffer, newSize, alignment);
#else
		if (newSize <= oldSize)
			return oldBuffer;
		else if (alignment <= alignof(std::max_align_t))
		{
			return realloc(oldBuffer, newSize);
		}

		void* buffer = std::aligned_alloc(alignment, newSize);
		if (oldBuffer)
		{
			if (buffer)
				memcpy(buffer, oldBuffer, std::min(oldSize, newSize));
			free(oldBuffer);
		}
		return buffer;
#endif
	}

	void RenderGraphRPS::RpsPrintCallback(void* pContext, const char* format, ...) noexcept
	{
		va_list args;
		va_start(args, format);
		RpsPrintVarCallback(pContext, format, args);
		va_end(args);
	}

	void RenderGraphRPS::RpsPrintVarCallback(void* pContext, const char* format, va_list vl) noexcept
	{
		constexpr char START_MSG[] = "RPS message: ";
		const S32 size = snprintf(nullptr, 0, format, vl) + 1;
		std::string buffer = START_MSG;
		buffer.resize(buffer.size() + size, ' ');
		snprintf(buffer.data() + sizeof(START_MSG) - 1, size, format, vl);
		Logger::InfoNoFile(buffer);
	}

	void RenderGraphRPS::RpsDeviceDestroyCallback(RpsDevice hDevice) noexcept
	{
	}

	void RenderGraphRPS::DefaultNodeCallback(const RpsCmdCallbackContext* pContext) noexcept
	{
		Logger::InfoNoFile("Unbound RPS node called.");
	}

	void Build()
	{
		RpsRenderGraphBuilder builder = RPS_NULL_HANDLE;

		RpsResourceDesc resDesc = {};
		resDesc.type = RPS_RESOURCE_TYPE_IMAGE_2D;
		resDesc.temporalLayers = 0;
		resDesc.flags = RPS_RESOURCE_FLAG_NONE;
		resDesc.image.width = 1024;
		resDesc.image.height = 896;
		resDesc.image.arrayLayers = 1;
		resDesc.image.mipLevels = 1;
		resDesc.image.format = RPS_FORMAT_R8G8B8A8_UINT;
		resDesc.image.sampleCount = 1;

		RpsResourceId id = rpsRenderGraphDeclareResource(builder, "Some_Buffer", 0, &resDesc);
	}

	RenderGraphRPS::RenderGraphRPS(GFX::Graphics& gfx)
	{
		ZE_RPS_ENABLE();

		// Create RPS device
		RpsDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.allocator.pfnAlloc = RpsAllocCallback;
		deviceCreateInfo.allocator.pfnFree = RpsFreeCallback;
		deviceCreateInfo.allocator.pfnRealloc = RpsReallocCallback;
		deviceCreateInfo.allocator.pContext = nullptr;
		deviceCreateInfo.printer.pfnPrintf = RpsPrintCallback;
		deviceCreateInfo.printer.pfnVPrintf = RpsPrintVarCallback;
		deviceCreateInfo.printer.pContext = nullptr;
		deviceCreateInfo.privateDataAllocInfo.size = sizeof(GFX::Device*);
		deviceCreateInfo.privateDataAllocInfo.alignment = 1;
		deviceCreateInfo.pfnDeviceOnDestroy = RpsDeviceDestroyCallback;
		ZE_RPS_THROW_FAILED(rpsDeviceCreate(&deviceCreateInfo, &rpsDevice));

		// Save device info in private data
		*reinterpret_cast<GFX::Device**>(rpsDeviceGetPrivateData(rpsDevice)) = &gfx.GetDevice();

		// Add debug logs
#if _ZE_MODE_DEBUG
		rpsSetGlobalDebugPrinter(&deviceCreateInfo.printer);
		rpsSetGlobalDebugPrinterLogLevel(RPS_DIAG_WARNING);
#endif

		RpsRenderGraphSignatureDesc graphSignatureDesc = {};
		graphSignatureDesc.numParams;
		graphSignatureDesc.numNodeDescs;
		graphSignatureDesc.maxExternalResources;
		graphSignatureDesc.pParamDescs;
		graphSignatureDesc.pNodeDescs;
		graphSignatureDesc.name = "Zenith Engine RPS Render Graph";

		RpsRenderGraphPhaseInfo phase = {};

		constexpr RpsQueueFlags QUEUE_FLAGS[] = { RPS_QUEUE_FLAG_GRAPHICS, RPS_QUEUE_FLAG_COMPUTE, RPS_QUEUE_FLAG_COPY };

		RpsRenderGraphCreateInfo graphCreateInfo = {};
		graphCreateInfo.scheduleInfo.scheduleFlags = RPS_SCHEDULE_UNSPECIFIED;
		graphCreateInfo.scheduleInfo.numQueues = sizeof(QUEUE_FLAGS) / sizeof(RpsQueueFlags);
		graphCreateInfo.scheduleInfo.pQueueInfos = QUEUE_FLAGS;
		graphCreateInfo.mainEntryCreateInfo.pSignatureDesc = &graphSignatureDesc;
		graphCreateInfo.mainEntryCreateInfo.hRpslEntryPoint = RPS_NULL_HANDLE;
		graphCreateInfo.mainEntryCreateInfo.defaultNodeCallback.pfnCallback = nullptr;
		graphCreateInfo.mainEntryCreateInfo.defaultNodeCallback.pUserContext = nullptr;
		graphCreateInfo.mainEntryCreateInfo.defaultNodeCallback.flags = RPS_CMD_CALLBACK_CUSTOM_ALL;
		graphCreateInfo.renderGraphFlags = RPS_RENDER_GRAPH_FLAG_NONE;
		graphCreateInfo.numPhases = 0;
		graphCreateInfo.pPhases = nullptr;

		ZE_RPS_THROW_FAILED(rpsRenderGraphCreate(rpsDevice, &graphCreateInfo, &renderGraph));
	}

	RenderGraphRPS::~RenderGraphRPS()
	{
		rpsRenderGraphDestroy(renderGraph);
		rpsDeviceDestroy(rpsDevice);
	}
}
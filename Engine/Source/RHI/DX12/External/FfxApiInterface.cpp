#if _ZE_FFX_API_ENABLED
#	include "RHI/DX12/External/FfxApiInterface.h"
ZE_WARNING_PUSH
#	include "dx12/ffx_api_dx12.h"
ZE_WARNING_POP

namespace ZE::RHI::DX12::External
{
	void FfxApiInterface::Destroy() noexcept
	{
		if (ffxApiDll)
		{
			[[maybe_unused]] const BOOL res = FreeLibrary(ffxApiDll);
			ZE_ASSERT(res, "Error unloading amd_fidelityfx_loader_dx12.dll!");
		}
	}

	void FfxApiInterface::MoveFrom(FfxApiInterface&& ffxInt) noexcept
	{
		ffxApiDll = std::exchange(ffxInt.ffxApiDll, nullptr);
		ffxCreateContext = ffxInt.ffxCreateContext;
		ffxFunctions = std::move(ffxInt.ffxFunctions);
	}

	Expected<FfxApiInterface> FfxApiInterface::Create() noexcept
	{
		FfxApiInterface ffxInt;
		ffxInt.ffxApiDll = LoadLibraryW(L"amd_fidelityfx_loader_dx12.dll");
		if (!ffxInt.ffxApiDll)
		{
			Status code = ZE_WIN_LAST_ERROR();
			ZE_CODE_ERROR(code, "Error loading amd_fidelityfx_loader_dx12.dll!");
			return std::unexpected(code);
		}

		ffxInt.ffxCreateContext = (PfnFfxCreateContext)GetProcAddress(ffxInt.ffxApiDll, "ffxCreateContext");
		ffxInt.ffxFunctions.DestroyContext = (PfnFfxDestroyContext)GetProcAddress(ffxInt.ffxApiDll, "ffxDestroyContext");
		ffxInt.ffxFunctions.Configure = (PfnFfxConfigure)GetProcAddress(ffxInt.ffxApiDll, "ffxConfigure");
		ffxInt.ffxFunctions.Query = (PfnFfxQuery)GetProcAddress(ffxInt.ffxApiDll, "ffxQuery");
		ffxInt.ffxFunctions.Dispatch = (PfnFfxDispatch)GetProcAddress(ffxInt.ffxApiDll, "ffxDispatch");

		if (ffxInt.ffxCreateContext == nullptr || ffxInt.ffxFunctions.DestroyContext == nullptr
			|| ffxInt.ffxFunctions.Configure == nullptr || ffxInt.ffxFunctions.Query == nullptr || ffxInt.ffxFunctions.Dispatch == nullptr)
		{
			Status code = ZE_WIN_LAST_ERROR();
			ZE_CODE_ERROR(code, "Error loading functions from amd_fidelityfx_loader_dx12.dll!");
			return std::unexpected(code);
		}
		return ffxInt;
	}

	ffxReturnCode_t FfxApiInterface::CreateFfxCtx(GFX::Device& dev, ffxContext* ctx, ffxCreateContextDescHeader& ctxHeader) const noexcept
	{
		if (ffxApiDll)
		{
			ffxCreateBackendDX12Desc backendDesc = { FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12, ctxHeader.pNext };
			backendDesc.device = dev.Get().dx12.GetDevice();

			ctxHeader.pNext = &backendDesc.header;
			ffxReturnCode_t ret = ffxCreateContext(ctx, &ctxHeader, nullptr);
			ctxHeader.pNext = backendDesc.header.pNext;

			return ret;
		}
		return FFX_API_RETURN_ERROR;
	}
}
#endif
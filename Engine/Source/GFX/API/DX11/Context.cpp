#include "GFX/API/DX11/Context.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11
{
	Context::Context(Device& dev, bool deffered)
	{
		ZE_GFX_ENABLE(dev);

		DX::ComPtr<ID3D11DeviceContext> tempCtx = nullptr;
		if (deffered)
		{
			ZE_GFX_THROW_FAILED(dev.GetDevice()->CreateDeferredContext(0, &tempCtx));
		}
		else
			dev.GetDevice()->GetImmediateContext(&tempCtx);

		ZE_GFX_THROW_FAILED(tempCtx->QueryInterface(IID_PPV_ARGS(&ctx)));
	}
}
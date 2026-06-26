#include "RHI/DX11/External/HbaoCtx.h"
#include "GFX/External/Error.h"

namespace ZE::RHI::DX11::External
{
	HbaoCtx::~HbaoCtx()
	{
		if (ctx)
			ctx->Release();
	}

	Expected<HbaoCtx> HbaoCtx::Create(GFX::Device& dev) noexcept
	{
		HbaoCtx hbao;

		GFSDK_SSAO_CustomHeap customHeap = {};
		customHeap.delete_ = ::operator delete;
		customHeap.new_ = ::operator new;

		ZE_HBAO_LOG_RET_FAILED_EXPECT(ZE_HBAO_ERROR(GFSDK_SSAO_CreateContext_D3D11(dev.Get().dx11.GetDevice(), &hbao.ctx, &customHeap)), "Failed to initialize HBAO+!");
		return hbao;
	}

	Status HbaoCtx::CreateResources(GFX::Device& dev, const GFSDK_SSAO_Parameters& params, UInt2 renderSize) noexcept
	{
		// Cmd queue is used mostly for resources cleanup so better to always use Main and see if it breaks things
		Status stat = ZE_HBAO_ERROR(ctx->PreCreateRTs(params, renderSize.X, renderSize.Y));
		// Flush any warnings from HBAO+ initialization
		[[maybe_unused]] HRESULT hr = S_OK;
		ZE_DX_CHECK_DEBUG_INFO(hr);
		return stat;
	}

	Status HbaoCtx::Render(GFX::Device& dev, GFX::CommandList& cl, GFX::Pipeline::FrameBuffer& buffers, const GFSDK_SSAO_Parameters& params,
		RID depth, RID normals, RID output, const Float4x4& projection, const Float4x4* viewTps, bool blendMultiply, bool linearDepth) noexcept
	{
		auto& framebuff = buffers.Get().dx11;

		GFSDK_SSAO_InputData_D3D11 hbaoInput = {};
		hbaoInput.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
		std::memcpy(hbaoInput.DepthData.ProjectionMatrix.Data.Array, &projection, sizeof(Float4x4));
		hbaoInput.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
		hbaoInput.DepthData.MetersToViewSpaceUnits = 1.0f;
		hbaoInput.DepthData.Viewport.Enable = false; // Just default viewport
		hbaoInput.DepthData.pFullResDepthTextureSRV = framebuff.GetSRV(depth);
		if (params.EnableDualLayerAO)
			hbaoInput.DepthData.pFullResDepthTexture2ndLayerSRV = framebuff.GetSRV(depth);

		if (normals != INVALID_RID)
		{
			ZE_ASSERT(viewTps, "View matrix is required when normals are provided");

			hbaoInput.NormalData.Enable = true;
			std::memcpy(hbaoInput.NormalData.WorldToViewMatrix.Data.Array, viewTps, sizeof(Float4x4));
			hbaoInput.NormalData.WorldToViewMatrix.Layout = GFSDK_SSAO_COLUMN_MAJOR_ORDER;
			hbaoInput.NormalData.DecodeScale = 1.0f;
			hbaoInput.NormalData.DecodeBias = 0.0f;
			hbaoInput.NormalData.pFullResNormalTextureSRV = framebuff.GetSRV(normals);
		}
		else
			hbaoInput.NormalData.Enable = false;

		GFSDK_SSAO_Output_D3D11 hbaoOutput = {};
		hbaoOutput.pRenderTargetView = framebuff.GetRTV(output);
		hbaoOutput.Blend.Mode = blendMultiply ? GFSDK_SSAO_MULTIPLY_RGB : GFSDK_SSAO_OVERWRITE_RGB;

		return ZE_HBAO_ERROR(ctx->RenderAO(cl.Get().dx11.GetContext(), hbaoInput, params, hbaoOutput, linearDepth ? GFSDK_SSAO_DRAW_AO : GFSDK_SSAO_RENDER_AO));
	}
}
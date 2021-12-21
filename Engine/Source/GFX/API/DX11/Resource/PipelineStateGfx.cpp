#include "GFX/API/DX11/Resource/PipelineStateGfx.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11::Resource
{
	PipelineStateGfx::PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Resource::DataBinding& binding)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx11);
		auto device = dev.Get().dx11.GetDevice();

		assert(desc.VS && "Vertex Shader is always required!");
		ID3DBlob* bytecode = desc.VS->Get().dx11.GetBytecode();
		ZE_GFX_THROW_FAILED(device->CreateVertexShader(bytecode->GetBufferPointer(),
			bytecode->GetBufferSize(), nullptr, &vertexShader));
		ZE_GFX_SET_ID(vertexShader, "VS");

		//std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc = vertexLayout->GetLayout();
		//ZE_GFX_THROW_FAILED(device->CreateInputLayout(layoutDesc.data(), static_cast<UINT>(layoutDesc.size()),
		//	bytecode->GetBufferPointer(), bytecode->GetBufferSize(), &inputLayout));
		//ZE_GFX_SET_ID(inputLayout, "Layout");

		if (desc.DS)
		{
			bytecode = desc.DS->Get().dx11.GetBytecode();
			ZE_GFX_THROW_FAILED(device->CreateDomainShader(bytecode->GetBufferPointer(),
				bytecode->GetBufferSize(), nullptr, &domainShader));
			ZE_GFX_SET_ID(domainShader, "DS");
		}
		if (desc.HS)
		{
			bytecode = desc.HS->Get().dx11.GetBytecode();
			ZE_GFX_THROW_FAILED(device->CreateHullShader(bytecode->GetBufferPointer(),
				bytecode->GetBufferSize(), nullptr, &hullShader));
			ZE_GFX_SET_ID(hullShader, "HS");
		}
		if (desc.GS)
		{
			bytecode = desc.GS->Get().dx11.GetBytecode();
			ZE_GFX_THROW_FAILED(device->CreateGeometryShader(bytecode->GetBufferPointer(),
				bytecode->GetBufferSize(), nullptr, &geometryShader));
			ZE_GFX_SET_ID(geometryShader, "GS");
		}
		if (desc.PS)
		{
			bytecode = desc.PS->Get().dx11.GetBytecode();
			ZE_GFX_THROW_FAILED(device->CreatePixelShader(bytecode->GetBufferPointer(),
				bytecode->GetBufferSize(), nullptr, &pixelShader));
			ZE_GFX_SET_ID(pixelShader, "PS");
		}

		D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT{});
		auto& blendTarget = blendDesc.RenderTarget[0];
		switch (desc.Blender)
		{
		case GFX::Resource::BlendType::Light:
		{
			blendTarget.BlendEnable = TRUE;
			blendTarget.SrcBlend = D3D11_BLEND_ONE;
			blendTarget.DestBlend = D3D11_BLEND_ONE;
			blendTarget.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
			break;
		}
		case GFX::Resource::BlendType::Normal:
		{
			blendTarget.BlendEnable = TRUE;
			blendTarget.SrcBlend = D3D11_BLEND_SRC_ALPHA; // Maybe ONE
			blendTarget.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			break;
		}
		}
		ZE_GFX_THROW_FAILED(device->CreateBlendState(&blendDesc, &blendState));
		ZE_GFX_SET_ID(blendState, "Blender");

		D3D11_DEPTH_STENCIL_DESC stencilDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT{});
		switch (desc.Stencil)
		{
		case GFX::Resource::StencilMode::Write:
		{
			stencilDesc.DepthEnable = FALSE;
			stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			stencilDesc.StencilEnable = TRUE;
			stencilDesc.StencilWriteMask = 0xFF;
			stencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			stencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
			break;
		}
		case GFX::Resource::StencilMode::Mask:
		{
			stencilDesc.DepthEnable = FALSE;
			stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			stencilDesc.StencilEnable = TRUE;
			stencilDesc.StencilReadMask = 0xFF;
			stencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
			stencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			break;
		}
		case GFX::Resource::StencilMode::DepthOff:
		{
			stencilDesc.DepthEnable = FALSE;
			stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			break;
		}
		case GFX::Resource::StencilMode::Reverse:
		{
			stencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
			break;
		}
		case GFX::Resource::StencilMode::DepthFirst:
		{
			stencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			break;
		}
		}
		ZE_GFX_THROW_FAILED(device->CreateDepthStencilState(&stencilDesc, &depthStencilState));
		ZE_GFX_SET_ID(depthStencilState, "Stencil");

		D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
		rasterDesc.CullMode = GetCulling(desc.Culling);
		rasterDesc.DepthClipEnable = desc.DepthClipEnable;
		ZE_GFX_THROW_FAILED(device->CreateRasterizerState(&rasterDesc, &rasterState));
		ZE_GFX_SET_ID(rasterState, "Raster");
	}

	void PipelineStateGfx::Bind(ID3D11DeviceContext4* ctx) const noexcept
	{
		ctx->IASetInputLayout(inputLayout.Get());
		ctx->VSSetShader(vertexShader.Get(), nullptr, 0);
		ctx->DSSetShader(domainShader.Get(), nullptr, 0);
		ctx->HSSetShader(hullShader.Get(), nullptr, 0);
		ctx->GSSetShader(geometryShader.Get(), nullptr, 0);
		ctx->PSSetShader(pixelShader.Get(), nullptr, 0);
		ctx->OMSetBlendState(blendState.Get(), nullptr, 0xFFFFFFFF);
		ctx->OMSetDepthStencilState(depthStencilState.Get(), 0xFF);
		ctx->RSSetState(rasterState.Get());
	}
}
#include "GFX/API/DX11/Resource/PipelineStateGfx.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11::Resource
{
	PipelineStateGfx::PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Binding::Schema& binding)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx11);
		auto device = dev.Get().dx11.GetDevice();
		topology = DX::GetTopology(desc.Topology, desc.Ordering);

		assert(desc.VS && "Vertex Shader is always required!");
		ID3DBlob* bytecode = desc.VS->Get().dx11.GetBytecode();
		ZE_GFX_THROW_FAILED(device->CreateVertexShader(bytecode->GetBufferPointer(),
			bytecode->GetBufferSize(), nullptr, &vertexShader));
		ZE_GFX_SET_ID(vertexShader, desc.VS->Get().dx11.GetName() + "_" + desc.DebugName);

		if (desc.InputLayout.size())
		{
			D3D11_INPUT_ELEMENT_DESC* elements = new D3D11_INPUT_ELEMENT_DESC[desc.InputLayout.size()];
			for (U32 i = 0; i < desc.InputLayout.size(); ++i)
			{
				GFX::Resource::InputParam paramType = desc.InputLayout.at(i);
				auto& element = elements[i];
				element.SemanticName = GFX::Resource::GetInputSemantic(paramType);
				element.SemanticIndex = 0;
				element.Format = DX::GetDXFormat(GFX::Resource::GetInputFormat(paramType));
				element.InputSlot = 0;
				element.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
				element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				element.InstanceDataStepRate = 0;
			}

			ZE_GFX_THROW_FAILED(device->CreateInputLayout(elements, static_cast<UINT>(desc.InputLayout.size()),
				bytecode->GetBufferPointer(), bytecode->GetBufferSize(), &inputLayout));
			ZE_GFX_SET_ID(inputLayout, "Layout_" + desc.DebugName);
			delete[] elements;
		}

		if (desc.DS)
		{
			bytecode = desc.DS->Get().dx11.GetBytecode();
			ZE_GFX_THROW_FAILED(device->CreateDomainShader(bytecode->GetBufferPointer(),
				bytecode->GetBufferSize(), nullptr, &domainShader));
			ZE_GFX_SET_ID(domainShader, desc.DS->Get().dx11.GetName() + "_" + desc.DebugName);
		}
		if (desc.HS)
		{
			bytecode = desc.HS->Get().dx11.GetBytecode();
			ZE_GFX_THROW_FAILED(device->CreateHullShader(bytecode->GetBufferPointer(),
				bytecode->GetBufferSize(), nullptr, &hullShader));
			ZE_GFX_SET_ID(hullShader, desc.HS->Get().dx11.GetName() + "_" + desc.DebugName);
		}
		if (desc.GS)
		{
			bytecode = desc.GS->Get().dx11.GetBytecode();
			ZE_GFX_THROW_FAILED(device->CreateGeometryShader(bytecode->GetBufferPointer(),
				bytecode->GetBufferSize(), nullptr, &geometryShader));
			ZE_GFX_SET_ID(geometryShader, desc.GS->Get().dx11.GetName() + "_" + desc.DebugName);
		}
		if (desc.PS)
		{
			bytecode = desc.PS->Get().dx11.GetBytecode();
			ZE_GFX_THROW_FAILED(device->CreatePixelShader(bytecode->GetBufferPointer(),
				bytecode->GetBufferSize(), nullptr, &pixelShader));
			ZE_GFX_SET_ID(pixelShader, desc.PS->Get().dx11.GetName() + "_" + desc.DebugName);
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
		default:
			break;
		}
		ZE_GFX_THROW_FAILED(device->CreateBlendState(&blendDesc, &blendState));
		ZE_GFX_SET_ID(blendState, "Blender_" + desc.DebugName);

		D3D11_DEPTH_STENCIL_DESC stencilDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT{});
		switch (desc.DepthStencil)
		{
		case GFX::Resource::DepthStencilMode::StencilWrite:
		{
			stencilDesc.DepthEnable = FALSE;
			stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			stencilDesc.StencilEnable = TRUE;
			stencilDesc.StencilWriteMask = 0xFF;
			stencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			stencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
			break;
		}
		case GFX::Resource::DepthStencilMode::StencilMask:
		{
			stencilDesc.DepthEnable = FALSE;
			stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			stencilDesc.StencilEnable = TRUE;
			stencilDesc.StencilReadMask = 0xFF;
			stencilDesc.StencilWriteMask = 0;
			stencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
			stencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			break;
		}
		case GFX::Resource::DepthStencilMode::DepthOff:
		{
			stencilDesc.DepthEnable = FALSE;
			stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			break;
		}
		case GFX::Resource::DepthStencilMode::DepthReverse:
		{
			stencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
			break;
		}
		case GFX::Resource::DepthStencilMode::DepthBefore:
		{
			stencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			break;
		}
		default:
			break;
		}
		ZE_GFX_THROW_FAILED(device->CreateDepthStencilState(&stencilDesc, &depthStencilState));
		ZE_GFX_SET_ID(depthStencilState, "DSS_" + desc.DebugName);

		D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
		rasterDesc.FillMode = desc.IsWireFrame() ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
		rasterDesc.CullMode = GetCulling(desc.Culling);
		rasterDesc.DepthClipEnable = desc.IsDepthClip();
		ZE_GFX_THROW_FAILED(device->CreateRasterizerState(&rasterDesc, &rasterState));
		ZE_GFX_SET_ID(rasterState, "Raster_" + desc.DebugName);
	}

	void PipelineStateGfx::Bind(ID3D11DeviceContext4* ctx) const noexcept
	{
		ctx->IASetPrimitiveTopology(topology);
		ctx->IASetInputLayout(inputLayout.Get());
		ctx->VSSetShader(vertexShader.Get(), nullptr, 0);
		ctx->DSSetShader(domainShader.Get(), nullptr, 0);
		ctx->HSSetShader(hullShader.Get(), nullptr, 0);
		ctx->GSSetShader(geometryShader.Get(), nullptr, 0);
		ctx->PSSetShader(pixelShader.Get(), nullptr, 0);
		ctx->OMSetBlendState(blendState.Get(), nullptr, 0xFFFFFFFF);
		SetStencilRef(ctx, 0);
		ctx->OMSetDepthStencilState(depthStencilState.Get(), 0xFF);
		ctx->RSSetState(rasterState.Get());
	}
}
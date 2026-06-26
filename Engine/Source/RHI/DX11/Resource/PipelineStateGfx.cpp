#include "RHI/DX11/Resource/PipelineStateGfx.h"

namespace ZE::RHI::DX11::Resource
{
	Expected<PipelineStateGfx> PipelineStateGfx::Create(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Binding::Schema& binding) noexcept
	{
		ZE_ASSERT(desc.VS, "Vertex Shader is always required!");

		PipelineStateGfx state;
		state.topology = DX::GetTopology(desc.Topology, desc.Ordering);

		auto device = dev.Get().dx11.GetDevice();
		ID3DBlob* bytecode = desc.VS->Get().dx11.GetBytecode();
		ZE_DX_RET_FAILED_EXPECT(device->CreateVertexShader(bytecode->GetBufferPointer(),
			bytecode->GetBufferSize(), nullptr, &state.vertexShader));
		ZE_DX_SET_ID(state.vertexShader, *desc.VS->Get().dx11.GetName() + "_" + desc.DebugName);

		if (desc.InputLayout.size())
		{
			auto elements = std::make_unique_for_overwrite<D3D11_INPUT_ELEMENT_DESC[]>(desc.InputLayout.size());
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

			ZE_DX_RET_FAILED_EXPECT(device->CreateInputLayout(elements.get(), Utils::SafeCast<UINT>(desc.InputLayout.size()),
				bytecode->GetBufferPointer(), bytecode->GetBufferSize(), &state.inputLayout));
			ZE_DX_SET_ID(state.inputLayout, "Layout_" + desc.DebugName);
		}

		if (desc.DS)
		{
			bytecode = desc.DS->Get().dx11.GetBytecode();
			ZE_DX_RET_FAILED_EXPECT(device->CreateDomainShader(bytecode->GetBufferPointer(),
				bytecode->GetBufferSize(), nullptr, &state.domainShader));
			ZE_DX_SET_ID(state.domainShader, *desc.DS->Get().dx11.GetName() + "_" + desc.DebugName);
		}
		if (desc.HS)
		{
			bytecode = desc.HS->Get().dx11.GetBytecode();
			ZE_DX_RET_FAILED_EXPECT(device->CreateHullShader(bytecode->GetBufferPointer(),
				bytecode->GetBufferSize(), nullptr, &state.hullShader));
			ZE_DX_SET_ID(state.hullShader, *desc.HS->Get().dx11.GetName() + "_" + desc.DebugName);
		}
		if (desc.GS)
		{
			bytecode = desc.GS->Get().dx11.GetBytecode();
			ZE_DX_RET_FAILED_EXPECT(device->CreateGeometryShader(bytecode->GetBufferPointer(),
				bytecode->GetBufferSize(), nullptr, &state.geometryShader));
			ZE_DX_SET_ID(state.geometryShader, *desc.GS->Get().dx11.GetName() + "_" + desc.DebugName);
		}
		if (desc.PS)
		{
			bytecode = desc.PS->Get().dx11.GetBytecode();
			ZE_DX_RET_FAILED_EXPECT(device->CreatePixelShader(bytecode->GetBufferPointer(),
				bytecode->GetBufferSize(), nullptr, &state.pixelShader));
			ZE_DX_SET_ID(state.pixelShader, *desc.PS->Get().dx11.GetName() + "_" + desc.DebugName);
		}

		D3D11_BLEND_DESC1 blendDesc = CD3D11_BLEND_DESC1(CD3D11_DEFAULT{});
		auto& blendTarget = blendDesc.RenderTarget[0];
		switch (desc.Blender)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::BlendType::None:
		{
			blendTarget.BlendEnable = FALSE;
			break;
		}
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
		ZE_DX_RET_FAILED_EXPECT(device->CreateBlendState1(&blendDesc, &state.blendState));
		ZE_DX_SET_ID(state.blendState, "Blender_" + desc.DebugName);

		D3D11_DEPTH_STENCIL_DESC stencilDesc = {};
		stencilDesc.DepthEnable = TRUE;
		stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		stencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
		stencilDesc.StencilEnable = FALSE;
		stencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		stencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		stencilDesc.FrontFace.StencilFailOp =
			stencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		stencilDesc.FrontFace.StencilDepthFailOp =
			stencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		stencilDesc.FrontFace.StencilPassOp =
			stencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		stencilDesc.FrontFace.StencilFunc =
			stencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		switch (desc.DepthStencil)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::DepthStencilMode::StencilOff:
			break;
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
			stencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
			break;
		}
		case GFX::Resource::DepthStencilMode::DepthBefore:
		{
			stencilDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
			stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			break;
		}
		}
		ZE_DX_RET_FAILED_EXPECT(device->CreateDepthStencilState(&stencilDesc, &state.depthStencilState));
		ZE_DX_SET_ID(state.depthStencilState, "DSS_" + desc.DebugName);

		D3D11_RASTERIZER_DESC2 rasterDesc = CD3D11_RASTERIZER_DESC2(CD3D11_DEFAULT{});
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		rasterDesc.FillMode = desc.IsWireframe() ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
		rasterDesc.CullMode = GetCulling(desc.Culling);
		rasterDesc.DepthClipEnable = desc.IsDepthClip();
		ZE_DX_RET_FAILED_EXPECT(device->CreateRasterizerState2(&rasterDesc, &state.rasterState));
		ZE_DX_SET_ID(state.rasterState, "Raster_" + desc.DebugName);

		return state;
	}

	void PipelineStateGfx::Bind(IDeviceContext* ctx) const noexcept
	{
		ctx->IASetPrimitiveTopology(topology);
		ctx->IASetInputLayout(inputLayout.Get());
		ctx->VSSetShader(vertexShader.Get(), nullptr, 0);
		ctx->DSSetShader(domainShader.Get(), nullptr, 0);
		ctx->HSSetShader(hullShader.Get(), nullptr, 0);
		ctx->GSSetShader(geometryShader.Get(), nullptr, 0);
		ctx->PSSetShader(pixelShader.Get(), nullptr, 0);
		ctx->OMSetBlendState(blendState.Get(), nullptr, UINT32_MAX);
		SetStencilRef(ctx, 0);
		ctx->OMSetDepthStencilState(depthStencilState.Get(), 0xFF);
		ctx->RSSetState(rasterState.Get());
	}
}
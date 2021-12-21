#include "GFX/API/DX12/Resource/PipelineStateGfx.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12::Resource
{
	PipelineStateGfx::PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Resource::DataBinding& binding)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx12);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC stateDesc;
		stateDesc.pRootSignature = binding.Get().dx12.GetSignature();

		assert(desc.VS && "Vertex Shader is always required!");
		ID3DBlob* bytecode = desc.VS->Get().dx12.GetBytecode();
		stateDesc.VS.pShaderBytecode = bytecode->GetBufferPointer();
		stateDesc.VS.BytecodeLength = bytecode->GetBufferSize();

		// Optional shaders
		if (desc.DS)
		{
			bytecode = desc.DS->Get().dx12.GetBytecode();
			stateDesc.DS.pShaderBytecode = bytecode->GetBufferPointer();
			stateDesc.DS.BytecodeLength = bytecode->GetBufferSize();
		}
		else
		{
			stateDesc.DS.pShaderBytecode = nullptr;
			stateDesc.DS.BytecodeLength = 0;
		}
		if (desc.HS)
		{
			bytecode = desc.HS->Get().dx12.GetBytecode();
			stateDesc.HS.pShaderBytecode = bytecode->GetBufferPointer();
			stateDesc.HS.BytecodeLength = bytecode->GetBufferSize();
		}
		else
		{
			stateDesc.HS.pShaderBytecode = nullptr;
			stateDesc.HS.BytecodeLength = 0;
		}
		if (desc.GS)
		{
			bytecode = desc.GS->Get().dx12.GetBytecode();
			stateDesc.GS.pShaderBytecode = bytecode->GetBufferPointer();
			stateDesc.GS.BytecodeLength = bytecode->GetBufferSize();
		}
		else
		{
			stateDesc.GS.pShaderBytecode = nullptr;
			stateDesc.GS.BytecodeLength = 0;
		}
		if (desc.PS)
		{
			bytecode = desc.PS->Get().dx12.GetBytecode();
			stateDesc.PS.pShaderBytecode = bytecode->GetBufferPointer();
			stateDesc.PS.BytecodeLength = bytecode->GetBufferSize();
		}
		else
		{
			stateDesc.PS.pShaderBytecode = nullptr;
			stateDesc.PS.BytecodeLength = 0;
		}

		// Stream Output stage
		stateDesc.StreamOutput = { 0 };

		// Blend state and blending sample mask (?)
		stateDesc.SampleMask = 0xFFFFFFFF;
		stateDesc.BlendState.AlphaToCoverageEnable = FALSE;
		stateDesc.BlendState.IndependentBlendEnable = FALSE;
		auto& blendTarget = stateDesc.BlendState.RenderTarget[0];
		switch (desc.Blender)
		{
		case GFX::Resource::BlendType::None:
		{
			blendTarget.BlendEnable = FALSE;
			blendTarget.LogicOpEnable = FALSE;
			blendTarget.RenderTargetWriteMask = 0;
			break;
		}
		case GFX::Resource::BlendType::Light:
		{
			blendTarget.BlendEnable = TRUE;
			blendTarget.SrcBlend = D3D12_BLEND_ONE;
			blendTarget.DestBlend = D3D12_BLEND_ONE;
			blendTarget.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_GREEN | D3D12_COLOR_WRITE_ENABLE_BLUE;
			break;
		}
		case GFX::Resource::BlendType::Normal:
		{
			blendTarget.BlendEnable = TRUE;
			blendTarget.SrcBlend = D3D12_BLEND_SRC_ALPHA; // Maybe ONE
			blendTarget.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			blendTarget.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			break;
		}
		}

		// Rasterizer state
		stateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		stateDesc.RasterizerState.CullMode = GetCulling(desc.Culling);
		stateDesc.RasterizerState.FrontCounterClockwise = FALSE;
		stateDesc.RasterizerState.DepthBias = 0;
		stateDesc.RasterizerState.DepthBiasClamp = 0.0f;
		stateDesc.RasterizerState.SlopeScaledDepthBias = 0.0f;
		stateDesc.RasterizerState.DepthClipEnable = desc.DepthClipEnable;
		stateDesc.RasterizerState.MultisampleEnable = FALSE;
		stateDesc.RasterizerState.AntialiasedLineEnable = FALSE;
		stateDesc.RasterizerState.ForcedSampleCount = 0;
		stateDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		// Depth-Stencil state
		switch (desc.Stencil)
		{
		case GFX::Resource::StencilMode::Off:
		{
			stateDesc.DepthStencilState.DepthEnable = TRUE;
			stateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			stateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
			stateDesc.DepthStencilState.StencilEnable = FALSE;
			break;
		}
		case GFX::Resource::StencilMode::Write:
		{
			stateDesc.DepthStencilState.DepthEnable = FALSE;
			stateDesc.DepthStencilState.StencilEnable = TRUE;
			stateDesc.DepthStencilState.StencilWriteMask = 0xFF;
			stateDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
			stateDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
			break;
		}
		case GFX::Resource::StencilMode::Mask:
		{
			stateDesc.DepthStencilState.DepthEnable = FALSE;
			stateDesc.DepthStencilState.StencilEnable = TRUE;
			stateDesc.DepthStencilState.StencilReadMask = 0xFF;
			stateDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
			stateDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
			break;
		}
		case GFX::Resource::StencilMode::DepthOff:
		{
			stateDesc.DepthStencilState.DepthEnable = FALSE;
			stateDesc.DepthStencilState.StencilEnable = FALSE;
			break;
		}
		case GFX::Resource::StencilMode::Reverse:
		{
			stateDesc.DepthStencilState.DepthEnable = TRUE;
			stateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
			stateDesc.DepthStencilState.StencilEnable = FALSE;
			break;
		}
		case GFX::Resource::StencilMode::DepthFirst:
		{
			stateDesc.DepthStencilState.DepthEnable = TRUE;
			stateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			stateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			stateDesc.DepthStencilState.StencilEnable = FALSE;
			break;
		}
		}

		stateDesc.InputLayout.pInputElementDescs = nullptr;
		stateDesc.InputLayout.NumElements = 0;

		// Input Layout description
		//std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc = vertexLayout->GetLayout();
		//ZE_GFX_THROW_FAILED(device->CreateInputLayout(layoutDesc.data(), static_cast<UINT>(layoutDesc.size()),
		//	bytecode->GetBufferPointer(), bytecode->GetBufferSize(), &inputLayout));
		//ZE_GFX_SET_ID(inputLayout, "Layout");

		// Cut value in strip topology indicating hole between vertices
		stateDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;

		// Topology type
		stateDesc.PrimitiveTopologyType = GetTopologyType(desc.Topology);

		// Output description
		assert(desc.RenderTargetsCount < 8);
		for (stateDesc.NumRenderTargets = 0; stateDesc.NumRenderTargets < desc.RenderTargetsCount; ++stateDesc.NumRenderTargets)
			stateDesc.RTVFormats[stateDesc.NumRenderTargets] = DX::GetDXFormat(desc.FormatsRT[stateDesc.NumRenderTargets]);
		stateDesc.DSVFormat = DX::GetDXFormat(desc.FormatDS);

		// Multisampling
		stateDesc.SampleDesc.Count = 1;
		stateDesc.SampleDesc.Quality = 0;

		stateDesc.NodeMask = 0;
		stateDesc.CachedPSO.pCachedBlob = nullptr;
		stateDesc.CachedPSO.CachedBlobSizeInBytes = 0;
		stateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		//ZE_GFX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateGraphicsPipelineState(&stateDesc, IID_PPV_ARGS(&state)));
	}
}
#include "GFX/API/DX12/Resource/PipelineStateGfx.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12::Resource
{
	PipelineStateGfx::PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx12);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC stateDesc;
		stateDesc.pRootSignature = nullptr; // TODO: Change later

		// Vertex Shader always present
		DX::ComPtr<ID3DBlob> bytecodeVS;
		ZE_GFX_THROW_FAILED(D3DReadFileToBlob((L"Shaders/" + desc.VS + L".cso").c_str(), &bytecodeVS));
		stateDesc.VS.pShaderBytecode = bytecodeVS->GetBufferPointer();
		stateDesc.VS.BytecodeLength = bytecodeVS->GetBufferSize();

		// Optional shaders
		DX::ComPtr<ID3DBlob> bytecodePS;
		if (desc.PS.empty())
		{
			stateDesc.PS.pShaderBytecode = nullptr;
			stateDesc.PS.BytecodeLength = 0;
		}
		else
		{
			ZE_GFX_THROW_FAILED(D3DReadFileToBlob((L"Shaders/" + desc.PS + L".cso").c_str(), &bytecodePS));
			stateDesc.PS.pShaderBytecode = bytecodePS->GetBufferPointer();
			stateDesc.PS.BytecodeLength = bytecodePS->GetBufferSize();
		}
		DX::ComPtr<ID3DBlob> bytecodeDS;
		if (desc.DS.empty())
		{
			stateDesc.DS.pShaderBytecode = nullptr;
			stateDesc.DS.BytecodeLength = 0;
		}
		else
		{
			ZE_GFX_THROW_FAILED(D3DReadFileToBlob((L"Shaders/" + desc.DS + L".cso").c_str(), &bytecodeDS));
			stateDesc.DS.pShaderBytecode = bytecodeDS->GetBufferPointer();
			stateDesc.DS.BytecodeLength = bytecodeDS->GetBufferSize();
		}
		DX::ComPtr<ID3DBlob> bytecodeHS;
		if (desc.HS.empty())
		{
			stateDesc.HS.pShaderBytecode = nullptr;
			stateDesc.HS.BytecodeLength = 0;
		}
		else
		{
			ZE_GFX_THROW_FAILED(D3DReadFileToBlob((L"Shaders/" + desc.HS + L".cso").c_str(), &bytecodeHS));
			stateDesc.HS.pShaderBytecode = bytecodeHS->GetBufferPointer();
			stateDesc.HS.BytecodeLength = bytecodeHS->GetBufferSize();
		}
		DX::ComPtr<ID3DBlob> bytecodeGS;
		if (desc.GS.empty())
		{
			stateDesc.GS.pShaderBytecode = nullptr;
			stateDesc.GS.BytecodeLength = 0;
		}
		else
		{
			ZE_GFX_THROW_FAILED(D3DReadFileToBlob((L"Shaders/" + desc.GS + L".cso").c_str(), &bytecodeGS));
			stateDesc.GS.pShaderBytecode = bytecodeGS->GetBufferPointer();
			stateDesc.GS.BytecodeLength = bytecodeGS->GetBufferSize();
		}

		// Stream Output stage
		stateDesc.StreamOutput = { 0 };

		// Blend state and blending sample mask (?)
		stateDesc.SampleMask = 0;
		stateDesc.BlendState.AlphaToCoverageEnable = FALSE;
		stateDesc.BlendState.IndependentBlendEnable = FALSE;
		auto& blendTarget = stateDesc.BlendState.RenderTarget[0];
		switch (desc.Blender)
		{
		case GFX::Resource::BlendType::Light:
		{
			blendTarget.BlendEnable = TRUE;
			blendTarget.SrcBlend = D3D12_BLEND_ONE;
			blendTarget.DestBlend = D3D12_BLEND_ONE;
			blendTarget.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_GREEN | D3D12_COLOR_WRITE_ENABLE_BLUE;
			break;
		}
		case GFX::Resource::BlendType::None:
		{
			blendTarget.BlendEnable = TRUE;
			blendTarget.SrcBlend = D3D12_BLEND_SRC_ALPHA; // Maybe ONE
			blendTarget.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			break;
		}
		}

		// Rasterizer state
		stateDesc.RasterizerState.CullMode = GetCulling(desc.Culling);
		stateDesc.RasterizerState.DepthClipEnable = desc.DepthEnable;

		// Depth-Stencil state
		switch (desc.Stencil)
		{
		case GFX::Resource::StencilMode::Write:
		{
			stateDesc.DepthStencilState.DepthEnable = FALSE;
			stateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			stateDesc.DepthStencilState.StencilEnable = TRUE;
			stateDesc.DepthStencilState.StencilWriteMask = 0xFF;
			stateDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
			stateDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
			break;
		}
		case GFX::Resource::StencilMode::Mask:
		{
			stateDesc.DepthStencilState.DepthEnable = FALSE;
			stateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			stateDesc.DepthStencilState.StencilEnable = TRUE;
			stateDesc.DepthStencilState.StencilReadMask = 0xFF;
			stateDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
			stateDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
			break;
		}
		case GFX::Resource::StencilMode::DepthOff:
		{
			stateDesc.DepthStencilState.DepthEnable = FALSE;
			stateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			break;
		}
		case GFX::Resource::StencilMode::Reverse:
		{
			stateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
			break;
		}
		case GFX::Resource::StencilMode::DepthFirst:
		{
			stateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			stateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			break;
		}
		}

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

		ZE_GFX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateGraphicsPipelineState(&stateDesc, IID_PPV_ARGS(&state)));
	}
}
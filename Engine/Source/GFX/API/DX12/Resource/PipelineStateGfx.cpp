#include "GFX/API/DX12/Resource/PipelineStateGfx.h"

namespace ZE::GFX::API::DX12::Resource
{
	PipelineStateGfx::PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Binding::Schema& binding)
	{
		ZE_DX_ENABLE_ID(dev.Get().dx12);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC stateDesc = {};
		stateDesc.pRootSignature = binding.Get().dx12.GetSignature();

		ZE_ASSERT(desc.VS, "Vertex Shader is always required!");
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

		// Blend state and blending sample mask
		stateDesc.SampleMask = UINT32_MAX;
		stateDesc.BlendState.AlphaToCoverageEnable = FALSE;
		stateDesc.BlendState.IndependentBlendEnable = FALSE;
		auto& blendTarget = stateDesc.BlendState.RenderTarget[0];
		// Default blend state
		blendTarget.LogicOpEnable = FALSE;
		blendTarget.SrcBlend = D3D12_BLEND_ONE;
		blendTarget.DestBlend = D3D12_BLEND_ZERO;
		blendTarget.BlendOp = D3D12_BLEND_OP_ADD;
		blendTarget.SrcBlendAlpha = D3D12_BLEND_ONE;
		blendTarget.DestBlendAlpha = D3D12_BLEND_ZERO;
		blendTarget.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendTarget.LogicOp = D3D12_LOGIC_OP_NOOP;
		blendTarget.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		switch (desc.Blender)
		{
		case GFX::Resource::BlendType::None:
		{
			blendTarget.BlendEnable = FALSE;
			break;
		}
		case GFX::Resource::BlendType::Light:
		{
			blendTarget.BlendEnable = TRUE;
			blendTarget.DestBlend = D3D12_BLEND_ONE;
			blendTarget.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_GREEN | D3D12_COLOR_WRITE_ENABLE_BLUE;
			break;
		}
		case GFX::Resource::BlendType::Normal:
		{
			blendTarget.BlendEnable = TRUE;
			blendTarget.SrcBlend = D3D12_BLEND_SRC_ALPHA; // Maybe ONE
			blendTarget.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			break;
		}
		}

		// Rasterizer state
		stateDesc.RasterizerState.FillMode = desc.IsWireframe() ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
		stateDesc.RasterizerState.CullMode = GetCulling(desc.Culling);
		stateDesc.RasterizerState.FrontCounterClockwise = FALSE;
		stateDesc.RasterizerState.DepthBias = 0;
		stateDesc.RasterizerState.DepthBiasClamp = 0.0f;
		stateDesc.RasterizerState.SlopeScaledDepthBias = 0.0f;
		stateDesc.RasterizerState.DepthClipEnable = desc.IsDepthClip();
		stateDesc.RasterizerState.MultisampleEnable = FALSE;
		stateDesc.RasterizerState.AntialiasedLineEnable = FALSE;
		stateDesc.RasterizerState.ForcedSampleCount = 0;
		stateDesc.RasterizerState.ConservativeRaster = desc.IsConservativeRaster() ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		// Depth-Stencil state defaults
		stateDesc.DepthStencilState.DepthEnable = TRUE;
		stateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		stateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		stateDesc.DepthStencilState.StencilEnable = FALSE;
		stateDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		stateDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		stateDesc.DepthStencilState.FrontFace.StencilFailOp =
			stateDesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		stateDesc.DepthStencilState.FrontFace.StencilDepthFailOp =
			stateDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		stateDesc.DepthStencilState.FrontFace.StencilPassOp =
			stateDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		stateDesc.DepthStencilState.FrontFace.StencilFunc =
			stateDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		// Proper definitions
		switch (desc.DepthStencil)
		{
		case GFX::Resource::DepthStencilMode::StencilOff:
		{
			stateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			break;
		}
		case GFX::Resource::DepthStencilMode::StencilWrite:
		{
			stateDesc.DepthStencilState.DepthEnable = FALSE;
			stateDesc.DepthStencilState.StencilEnable = TRUE;
			stateDesc.DepthStencilState.StencilWriteMask = 0xFF;
			stateDesc.DepthStencilState.FrontFace.StencilPassOp =
				stateDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
			break;
		}
		case GFX::Resource::DepthStencilMode::StencilMask:
		{
			stateDesc.DepthStencilState.DepthEnable = FALSE;
			stateDesc.DepthStencilState.StencilEnable = TRUE;
			stateDesc.DepthStencilState.StencilReadMask = 0xFF;
			stateDesc.DepthStencilState.StencilWriteMask = 0;
			stateDesc.DepthStencilState.FrontFace.StencilFunc =
				stateDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
			break;
		}
		case GFX::Resource::DepthStencilMode::DepthOff:
		{
			stateDesc.DepthStencilState.DepthEnable = FALSE;
			break;
		}
		case GFX::Resource::DepthStencilMode::DepthReverse:
		{
			stateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			stateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
			break;
		}
		case GFX::Resource::DepthStencilMode::DepthBefore:
		{
			stateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			break;
		}
		}

		// Input Layout
		stateDesc.InputLayout.NumElements = Utils::SafeCast<U32>(desc.InputLayout.size());
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements(stateDesc.InputLayout.NumElements);
		if (stateDesc.InputLayout.NumElements)
		{
			stateDesc.InputLayout.pInputElementDescs = inputElements.data();
			for (U32 i = 0; i < stateDesc.InputLayout.NumElements; ++i)
			{
				GFX::Resource::InputParam paramType = desc.InputLayout.at(i);
				auto& element = inputElements.at(i);
				element.SemanticName = GFX::Resource::GetInputSemantic(paramType);
				element.SemanticIndex = 0;
				element.Format = DX::GetDXFormat(GFX::Resource::GetInputFormat(paramType));
				element.InputSlot = 0;
				element.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
				element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				element.InstanceDataStepRate = 0;
			}
		}
		else
			stateDesc.InputLayout.pInputElementDescs = nullptr;

		// Cut value in strip topology indicating hole between vertices
		stateDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;

		// Topology type
		stateDesc.PrimitiveTopologyType = GetTopologyType(desc.Topology);
		topology = DX::GetTopology(desc.Topology, desc.Ordering);

		// Output description
		ZE_ASSERT(desc.RenderTargetsCount < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT, "Too many render targets!");
		for (stateDesc.NumRenderTargets = 0; stateDesc.NumRenderTargets < desc.RenderTargetsCount; ++stateDesc.NumRenderTargets)
			stateDesc.RTVFormats[stateDesc.NumRenderTargets] = DX::GetDXFormat(desc.FormatsRT[stateDesc.NumRenderTargets]);
		for (U8 i = Utils::SafeCast<U8>(stateDesc.NumRenderTargets); i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			stateDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
		stateDesc.DSVFormat = DX::GetTypedDepthDXFormat(desc.FormatDS);

		// Multisampling
		stateDesc.SampleDesc.Count = 1;
		stateDesc.SampleDesc.Quality = 0;

		stateDesc.NodeMask = 0;
		stateDesc.CachedPSO.pCachedBlob = nullptr;
		stateDesc.CachedPSO.CachedBlobSizeInBytes = 0;
		stateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		ZE_DX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateGraphicsPipelineState(&stateDesc, IID_PPV_ARGS(&state)));
		ZE_DX_SET_ID(state, "PSO_" + desc.DebugName);
	}

	void PipelineStateGfx::Bind(GFX::CommandList& cl) const noexcept
	{
		auto list = cl.Get().dx12.GetList();
		list->SetPipelineState(GetState());
		list->IASetPrimitiveTopology(topology);
		SetStencilRef(cl, 0);
	}
}
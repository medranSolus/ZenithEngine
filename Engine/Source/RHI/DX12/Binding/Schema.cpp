#include "RHI/DX12/Binding/Schema.h"

namespace ZE::RHI::DX12::Binding
{
	Schema::Schema(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc)
	{
		ZE_DX_ENABLE(dev.Get().dx12);

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC signatureDesc = {};
		signatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		signatureDesc.Desc_1_1.NumStaticSamplers = Utils::SafeCast<U32>(desc.Samplers.size());
		std::unique_ptr < D3D12_STATIC_SAMPLER_DESC[] > staticSamplers = nullptr;
		if (signatureDesc.Desc_1_1.NumStaticSamplers)
		{
			staticSamplers = std::make_unique<D3D12_STATIC_SAMPLER_DESC[]>(signatureDesc.Desc_1_1.NumStaticSamplers);
			signatureDesc.Desc_1_1.pStaticSamplers = staticSamplers.get();
			// Load data for samplers
			for (U32 i = 0; const auto& samplerDesc : desc.Samplers)
			{
				auto& sampler = staticSamplers[i++];
				sampler.Filter = GetFilterType(samplerDesc.Type);
				sampler.AddressU = GetTextureAddressMode(samplerDesc.Address.U);
				sampler.AddressV = GetTextureAddressMode(samplerDesc.Address.V);
				sampler.AddressW = GetTextureAddressMode(samplerDesc.Address.W);
				sampler.MipLODBias = samplerDesc.MipLevelBias;
				sampler.MaxAnisotropy = samplerDesc.MaxAnisotropy;
				sampler.ComparisonFunc = GetComparisonFunc(samplerDesc.Comparison);
				sampler.BorderColor = GetStaticBorderColor(samplerDesc.EdgeColor);
				sampler.MinLOD = samplerDesc.MinLOD;
				sampler.MaxLOD = samplerDesc.MaxLOD;
				sampler.ShaderRegister = samplerDesc.Slot;
				sampler.RegisterSpace = 0;
				sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			}
		}

		// Check input data
		signatureDesc.Desc_1_1.NumParameters = 0;
		for (const auto& entry : desc.Ranges)
		{
			entry.Validate();

			if (entry.Flags & GFX::Binding::RangeFlag::Constant
				|| entry.Flags & GFX::Binding::RangeFlag::BufferPack)
				++signatureDesc.Desc_1_1.NumParameters;
			else if (!(entry.Flags & GFX::Binding::RangeFlag::BufferPackAppend))
				signatureDesc.Desc_1_1.NumParameters += entry.Count;
		}
		count = signatureDesc.Desc_1_1.NumParameters;
		bindings = new BindType[count];
		auto parameters = std::make_unique<D3D12_ROOT_PARAMETER1[]>(count);
		signatureDesc.Desc_1_1.pParameters = parameters.get();

		// Location | Descs
		std::vector<std::pair<U32, std::vector<D3D12_DESCRIPTOR_RANGE1>>> tables;
		GFX::ShaderPresenceMask shaderPresence;
		GFX::ShaderPresenceMask shaderPresenceConstants;
		// Fill signature parameters
		for (U32 i = 0; const auto& entry : desc.Ranges)
		{
			if (entry.Flags & GFX::Binding::RangeFlag::BufferPackAppend)
			{
				ZE_ASSERT(i > 0 && parameters[i - 1].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
					"New ranges for table must be specified directly after that table! Preceding range must also have BufferPackAppend or BufferPack flag!");
				ZE_ASSERT(entry.StartSlot + entry.Count < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT,
					"Too much shader slots!");

				auto& range = tables.back().second.emplace_back();
				if (entry.Flags & GFX::Binding::RangeFlag::SRV)
					range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				else if (entry.Flags & GFX::Binding::RangeFlag::UAV)
					range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				else
					range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				range.NumDescriptors = entry.Count;
				range.BaseShaderRegister = entry.StartSlot;
				range.RegisterSpace = GetRegisterSpaceForShader(entry.Flags, entry.Shaders);
				// When descriptors are set in descriptor heap they won't change until draw finishes
				// and when data is entering pipeline it is already static
				if (entry.Flags & GFX::Binding::RangeFlag::StaticData)
					range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
				else
					range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;
				range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			}
			else
			{
				auto& parameter = parameters[i];
				parameter.ShaderVisibility = GetShaderVisibility(entry.Shaders, &shaderPresence);
				if (entry.Flags & GFX::Binding::RangeFlag::Constant)
				{
					ZE_ASSERT(entry.StartSlot < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT, "Too much shader slots!");
					ZE_ASSERT(!shaderPresenceConstants.SetPresence(entry.Shaders), "Only single Constant per shader type is allowed!");

					parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
					parameter.Constants.ShaderRegister = entry.StartSlot;
					parameter.Constants.RegisterSpace = GetRegisterSpaceForShader(entry.Flags, entry.Shaders);
					parameter.Constants.Num32BitValues = entry.Count / sizeof(U32) + static_cast<bool>(entry.Count % sizeof(U32));
					bindings[i++] = BindType::Constant;
				}
				else if (entry.Flags & GFX::Binding::RangeFlag::BufferPack)
				{
					ZE_ASSERT(entry.StartSlot + entry.Count < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT,
						"Too much shader slots!");

					parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					tables.emplace_back(i, std::vector<D3D12_DESCRIPTOR_RANGE1>{ 1 });
					bindings[i++] = BindType::Table;

					auto& range = tables.back().second.front();
					if (entry.Flags & GFX::Binding::RangeFlag::SRV)
						range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					else if (entry.Flags & GFX::Binding::RangeFlag::UAV)
						range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
					else
						range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					range.NumDescriptors = entry.Count;
					range.BaseShaderRegister = entry.StartSlot;
					range.RegisterSpace = GetRegisterSpaceForShader(entry.Flags, entry.Shaders);
					// When descriptors are set in descriptor heap they won't change until draw finishes
					// and when data is entering pipeline it is already static
					if (entry.Flags & GFX::Binding::RangeFlag::StaticData)
						range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
					else
						range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;
					range.OffsetInDescriptorsFromTableStart = 0;
				}
				else
				{
					ZE_ASSERT(entry.StartSlot + entry.Count < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT,
						"Too much shader slots!");

					BindType type;
					if (entry.Flags & GFX::Binding::RangeFlag::SRV)
					{
						parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
						type = BindType::SRV;
					}
					else if (entry.Flags & GFX::Binding::RangeFlag::UAV)
					{
						parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
						type = BindType::UAV;
					}
					else
					{
						parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
						type = BindType::CBV;
					}
					parameter.Descriptor.ShaderRegister = entry.StartSlot;
					parameter.Descriptor.RegisterSpace = GetRegisterSpaceForShader(entry.Flags, entry.Shaders);
					if (entry.Flags & GFX::Binding::RangeFlag::StaticData || entry.Flags & GFX::Binding::RangeFlag::CBV)
						parameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC;
					else
						parameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;

					bindings[i++] = type;
					for (U32 j = 1; j < entry.Count; ++j)
					{
						bindings[i] = type;
						auto& nextParameter = parameters[i++];
						nextParameter.ShaderVisibility = parameter.ShaderVisibility;
						nextParameter.ParameterType = parameter.ParameterType;
						nextParameter.Descriptor.ShaderRegister = parameter.Descriptor.ShaderRegister + j;
						nextParameter.Descriptor.RegisterSpace = parameter.Descriptor.RegisterSpace;
						nextParameter.Descriptor.Flags = parameter.Descriptor.Flags;
					}
				}
			}
		}
		// Finish filling descriptor tables
		for (const auto& table : tables)
		{
			ZE_ASSERT(parameters[table.first].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				"Trying to setup ranges for non descriptor table type!");
			parameters[table.first].DescriptorTable.NumDescriptorRanges = Utils::SafeCast<UINT>(table.second.size());
			parameters[table.first].DescriptorTable.pDescriptorRanges = table.second.data();
		}

		signatureDesc.Desc_1_1.Flags =
			D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;
		if (shaderPresence.IsCompute())
		{
			ZE_ASSERT(!shaderPresence.IsGfx(),
				"Compute Shader binding detected alongside other shaders resulting in disabling all other graphics shader stages. Check creation of the SchemaDesc!");
			signatureDesc.Desc_1_1.Flags |=
				D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
			isCompute = true;
		}
		else
		{
			if (!shaderPresence.IsVertex())
				signatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
			if (!shaderPresence.IsDomain())
				signatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
			if (!shaderPresence.IsHull())
				signatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
			if (!shaderPresence.IsGeometry())
				signatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
			if (!shaderPresence.IsPixel())
				signatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
			isCompute = false;
		}
		if (!(desc.Options & GFX::Binding::SchemaOption::NoVertexBuffer) || shaderPresence.IsCompute())
			signatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		if (desc.Options & GFX::Binding::SchemaOption::AllowStreamOutput)
		{
			ZE_ASSERT(!shaderPresence.IsCompute(), "Stream output is not accessible in Compute Shader!");
			signatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
		}

		// Sanity check for Signature 1.1, if there is no support for new one there should be code to handle that, for now assume 1.1
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = { D3D_ROOT_SIGNATURE_VERSION_1_1 };
		ZE_DX_THROW_FAILED(dev.Get().dx12.GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)));

		DX::ComPtr<ID3DBlob> serializedSignature = nullptr;
		DX::ComPtr<ID3DBlob> errors = nullptr;
		ZE_DX_SET_DEBUG_WATCH();
		ZE_WIN_EXCEPT_RESULT = D3D12SerializeVersionedRootSignature(&signatureDesc, &serializedSignature, &errors);
		if (FAILED(ZE_WIN_EXCEPT_RESULT))
			throw Exception::GenericException(__LINE__, __FILENAME__, reinterpret_cast<char*>(errors->GetBufferPointer()), "Root Signature Invalid Parameter");
		ZE_DX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateRootSignature(0,
			serializedSignature->GetBufferPointer(), serializedSignature->GetBufferSize(), IID_PPV_ARGS(&signature)));
	}
}
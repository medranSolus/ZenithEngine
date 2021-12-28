#include "GFX/API/DX12/Material/Schema.h"
#include "GFX/API/DX/GraphicsException.h"
#include <sstream>

namespace ZE::GFX::API::DX12::Material
{
	Schema::Schema(GFX::Device& dev, const GFX::Material::SchemaDesc& desc)
	{
		ZE_ASSERT(desc.Ranges.size() > 0, "Empty SchemaDesc!");
		ZE_GFX_ENABLE(dev.Get().dx12);

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC signatureDesc;
		signatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		signatureDesc.Desc_1_1.NumStaticSamplers = static_cast<U32>(desc.Samplers.size());
		D3D12_STATIC_SAMPLER_DESC* staticSamplers = new D3D12_STATIC_SAMPLER_DESC[signatureDesc.Desc_1_1.NumStaticSamplers];
		signatureDesc.Desc_1_1.pStaticSamplers = staticSamplers;
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

		// Check input data and setup samplers visibility
		signatureDesc.Desc_1_1.NumParameters = 0;
		for (const auto& entry : desc.Ranges)
		{
			ZE_ASSERT(((entry.Flags & MaterialFlags::Constant) == 0 || (entry.Flags & MaterialFlags::Material) == 0)
				&& ((entry.Flags & MaterialFlags::Constant) == 0 || (entry.Flags & MaterialFlags::MaterialAppend) == 0)
				&& ((entry.Flags & MaterialFlags::Material) == 0 || (entry.Flags & MaterialFlags::MaterialAppend) == 0),
				"Single range should only have one of the flags: Constant, Material or MaterialAppend!");
			ZE_ASSERT((entry.Flags & MaterialFlags::Constant) == 0
				|| (entry.Flags & (MaterialFlags::SRV | MaterialFlags::UAV | MaterialFlags::CBV)) == 0,
				"Flags SRV, UAV, CBV or Samplers cannot be specified with flag Constant!");
			ZE_ASSERT(((entry.Flags & MaterialFlags::SRV) != 0)
				!= ((entry.Flags & MaterialFlags::UAV) != 0)
				!= ((entry.Flags & MaterialFlags::CBV) != 0),
				"Single range should only have one of the flags: SRV, UAV, CBV or Samplers!");
			ZE_ASSERT(entry.Count != 0, "Count should be at least 1!");

			if (entry.Flags & MaterialFlags::Constant
				|| entry.Flags & MaterialFlags::Material)
				++signatureDesc.Desc_1_1.NumParameters;
			else if (!(entry.Flags & MaterialFlags::MaterialAppend))
				signatureDesc.Desc_1_1.NumParameters += entry.Count;
		}
		D3D12_ROOT_PARAMETER1* parameters = new D3D12_ROOT_PARAMETER1[signatureDesc.Desc_1_1.NumParameters];
		signatureDesc.Desc_1_1.pParameters = parameters;

		// Location | Size | Desc
		std::vector<std::pair<U32, std::pair<U32, D3D12_DESCRIPTOR_RANGE1*>>> tables;
		std::bitset<6> shaderPresence(0);
		// Fill signature parameters
		for (U32 i = 0; const auto & entry : desc.Ranges)
		{
			if (entry.Flags & MaterialFlags::MaterialAppend)
			{
				ZE_ASSERT(i > 0 && parameters[i - 1].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
					"New ranges for table must be specified directly after that table! Preceeding range must also have MaterialAppend or Material flag!");
				tables.back().second.second = reinterpret_cast<D3D12_DESCRIPTOR_RANGE1*>(realloc(tables.back().second.second,
					++tables.back().second.first * sizeof(D3D12_DESCRIPTOR_RANGE1)));

				auto& range = tables.back().second.second[tables.back().second.first - 1];
				if (entry.Flags & MaterialFlags::SRV)
					range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				else if (entry.Flags & MaterialFlags::UAV)
					range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				else
					range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				range.NumDescriptors = entry.Count;
				range.BaseShaderRegister = entry.SlotStart;
				range.RegisterSpace = GetRegisterSpaceForShader(entry.Shader);
				// When descriptors are set in descriptor heap they won't change until draw finishes and when data is entering pipeline it is already static
				range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
				range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			}
			else
			{
				auto& parameter = parameters[i];
				parameter.ShaderVisibility = GetShaderVisibility(entry.Shader, &shaderPresence);
				if (entry.Flags & MaterialFlags::Constant)
				{
					parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
					parameter.Constants.ShaderRegister = entry.SlotStart;
					parameter.Constants.RegisterSpace = GetRegisterSpaceForShader(entry.Shader);
					parameter.Constants.Num32BitValues = entry.Count / sizeof(U32) + static_cast<bool>(entry.Count % sizeof(U32));
					++i;
				}
				else if (entry.Flags & MaterialFlags::Material)
				{
					parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					tables.emplace_back(i++, std::make_pair(1, reinterpret_cast<D3D12_DESCRIPTOR_RANGE1*>(malloc(sizeof(D3D12_DESCRIPTOR_RANGE1)))));

					auto& range = tables.back().second.second[0];
					if (entry.Flags & MaterialFlags::SRV)
						range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					else if (entry.Flags & MaterialFlags::UAV)
						range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
					else
						range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					range.NumDescriptors = entry.Count;
					range.BaseShaderRegister = entry.SlotStart;
					range.RegisterSpace = GetRegisterSpaceForShader(entry.Shader);
					// When descriptors are set in descriptor heap they won't change until draw finishes and when data is entering pipeline it is already static
					range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
					range.OffsetInDescriptorsFromTableStart = 0;
				}
				else
				{
					if (entry.Flags & MaterialFlags::SRV)
						parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
					else if (entry.Flags & MaterialFlags::UAV)
						parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
					else
						parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
					parameter.Descriptor.ShaderRegister = entry.SlotStart;
					parameter.Descriptor.RegisterSpace = GetRegisterSpaceForShader(entry.Shader);
					parameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC;
					++i;
					for (U32 j = 1; j < entry.Count;)
					{
						auto& nextParameter = parameters[i++];
						nextParameter.ShaderVisibility = parameter.ShaderVisibility;
						nextParameter.ParameterType = parameter.ParameterType;
						++j;
						nextParameter.Descriptor.ShaderRegister = parameter.Descriptor.ShaderRegister + j;
						nextParameter.Descriptor.RegisterSpace = parameter.Descriptor.RegisterSpace;
						nextParameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC;
					}
				}
			}
		}
		// Finish filling descriptor tables
		for (const auto& table : tables)
		{
			ZE_ASSERT(parameters[table.first].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				"Trying to setup ranges for non descriptor table type!");
			parameters[table.first].DescriptorTable.NumDescriptorRanges = table.second.first;
			parameters[table.first].DescriptorTable.pDescriptorRanges = table.second.second;
		}

		signatureDesc.Desc_1_1.Flags =
			D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;
		if (shaderPresence[5])
		{
			ZE_ASSERT(!shaderPresence[0] && !shaderPresence[1] && !shaderPresence[2] && !shaderPresence[3] && !shaderPresence[4],
				"Compute Shader binding detected alongside other shaders resulting in disabling all other graphics shader stages. Check creation of the DataBindingDesc!");
			signatureDesc.Desc_1_1.Flags |=
				D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
		}
		else
		{
			if (!shaderPresence[0])
				signatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
			if (!shaderPresence[1])
				signatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
			if (!shaderPresence[2])
				signatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
			if (!shaderPresence[3])
				signatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
			if (!shaderPresence[4])
				signatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
		}
		if (!(desc.Options & MaterialOptions::NoVertexBuffer) || shaderPresence[5])
			signatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		if (desc.Options & MaterialOptions::AllowStreamOutput)
		{
			ZE_ASSERT(!shaderPresence[5], "Stream output is not accessible in Compute Shader!");
			signatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
		}

		// Sanity check for Signature 1.1, if there is no support for new one there should be code to handle that, for now assume 1.1
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = { D3D_ROOT_SIGNATURE_VERSION_1_1 };
		ZE_GFX_THROW_FAILED(dev.Get().dx12.GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)));

		DX::ComPtr<ID3DBlob> serializedSignature;
		DX::ComPtr<ID3DBlob> errors;
		ZE_GFX_SET_DEBUG_WATCH();
		ZE_WIN_EXCEPT_RESULT = D3D12SerializeVersionedRootSignature(&signatureDesc, &serializedSignature, &errors);
		if (FAILED(ZE_WIN_EXCEPT_RESULT))
			throw Exception::GenericException(__LINE__, __FILENAME__, reinterpret_cast<char*>(errors->GetBufferPointer()), "Root Signature Invalid Parameter");
		ZE_GFX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateRootSignature(0,
			serializedSignature->GetBufferPointer(), serializedSignature->GetBufferSize(), IID_PPV_ARGS(&signature)));

		for (auto& table : tables)
			free(table.second.second);
		delete[] parameters;
		delete[] staticSamplers;
	}
}
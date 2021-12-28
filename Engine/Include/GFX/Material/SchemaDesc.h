#pragma once
#include "GFX/Resource/SamplerDesc.h"
#include "GFX/Resource/ShaderType.h"
#include "BindingRange.h"

namespace ZE::GFX::Material
{
	// Options to create shader signature with
	enum BindingOptions : U8
	{
		NoVertexBuffer = 1,
		AllowStreamOutput = 2
	};

	// Descriptor for creation of Schema
	struct SchemaDesc
	{
		U8 Options = 0;
		std::vector<BindingRange> Ranges;
		std::vector<Resource::SamplerDesc> Samplers;

		void AddRange(BindingRange&& range) noexcept { Ranges.emplace_back(std::forward<BindingRange>(range)); }
		void AddSampler(Resource::SamplerDesc&& sampler) noexcept { Samplers.emplace_back(std::forward<Resource::SamplerDesc>(sampler)); }
		void Append(const SchemaDesc& binding) noexcept
		{
			Ranges.insert(Ranges.end(), binding.Ranges.begin(), binding.Ranges.end());
			Samplers.insert(Samplers.end(), binding.Samplers.begin(), binding.Samplers.end());
		}
	};
}
namespace ZE
{
	// Options to create shader signature with
	typedef GFX::Material::BindingOptions MaterialOptions;
}
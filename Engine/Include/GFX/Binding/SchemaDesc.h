#pragma once
#include "GFX/Resource/SamplerDesc.h"
#include "GFX/Resource/ShaderType.h"
#include "Range.h"

namespace ZE::GFX::Binding
{
	typedef U8 SchemaOptions;
	// Options to create shader signature with
	enum SchemaOption : SchemaOptions
	{
		NoVertexBuffer = 1,
		AllowStreamOutput = 2
	};

	// Descriptor for creation of Schema
	struct SchemaDesc
	{
		SchemaOptions Options = 0;
		std::vector<Range> Ranges;
		std::vector<Resource::SamplerDesc> Samplers;

		void AddRange(Range&& range) noexcept { Ranges.emplace_back(std::forward<Range>(range)); }
		void AddSampler(Resource::SamplerDesc&& sampler) noexcept { Samplers.emplace_back(std::forward<Resource::SamplerDesc>(sampler)); }
		void Append(const SchemaDesc& binding) noexcept
		{
			Ranges.insert(Ranges.end(), binding.Ranges.begin(), binding.Ranges.end());
			Samplers.insert(Samplers.end(), binding.Samplers.begin(), binding.Samplers.end());
		}
	};
}
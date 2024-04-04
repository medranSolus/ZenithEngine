#pragma once
#include "GFX/Resource/SamplerDesc.h"
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

		void Clear() noexcept { Options = 0; Ranges.clear(); Samplers.clear(); }
		void AddRange(Range&& range, Resource::ShaderTypes useShaders = Resource::ShaderType::All) noexcept { Ranges.emplace_back(std::forward<Range>(range)).Shaders &= useShaders; }
		void AddRange(const Range& range, Resource::ShaderTypes useShaders = Resource::ShaderType::All) noexcept { Ranges.emplace_back(range).Shaders &= useShaders; }
		void AddSampler(Resource::SamplerDesc&& sampler) noexcept { Samplers.emplace_back(std::forward<Resource::SamplerDesc>(sampler)); }

		void Append(const SchemaDesc& binding, Resource::ShaderTypes useShaders) noexcept { AppendRanges(binding.Ranges, useShaders); AppendSamplers(binding.Samplers); }
		void AppendRanges(const SchemaDesc& binding, Resource::ShaderTypes useShaders) noexcept { AppendRanges(binding.Ranges, useShaders); }
		void AppendSamplers(const SchemaDesc& binding) noexcept { AppendSamplers(binding.Samplers); }

		void AppendRanges(const std::vector<Range>& ranges, Resource::ShaderTypes useShaders) noexcept;
		void AppendSamplers(const std::vector<Resource::SamplerDesc>& samplers) noexcept;
	};
}
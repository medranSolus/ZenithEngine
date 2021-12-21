#pragma once
#include "SamplerDesc.h"
#include "ShaderType.h"

namespace ZE::GFX::Resource
{
	// List of default pass type IDs used in data binding. To use DataBinding in custom render pass
	// always start your enumeration with DataBindingDefaultID::Count value to avoid any ID collision
	enum BindingDefaultID : U32
	{
		LightCombine,
		Count
	};

	// Flags changing behavior of specific range
	enum BindingRangeFlags : U8
	{
		// Interpret range as inlined constant, Count field should contain size of structure to be used with shader
		Constant = 1,
		// Range will be used as material slot. Later during material binding, order of the materials
		// should be preserved according to the order of the ranges with this parameter.
		// In addition to this flag, there should be also only one of the following flags:
		// SRV, UAV, CBV, Samplers
		Material = 2,
		// Appends new range to the previous material. Must follow directly after range with same or 'Material' flag
		MaterialAppend = 4,
		// Mark range as Shader Resource View range. Cannot use with Constant flag set
		SRV = 8,
		// Mark range as Unordered Access View range. Cannot use with Constant flag set
		UAV = 16,
		// Mark range as Constant Buffer View range. Cannot use with Constant flag set
		CBV = 32
	};

	// Single range of shader registers to use
	struct BindingRange
	{
		U32 Count;
		U32 SlotStart;
		ShaderType Shader;
		U8 Flags = 0;
	};

	// Options to create data binding with
	enum BindingOptions : U8
	{
		NoVertexBuffer = 1,
		AllowStreamOutput = 2
	};

	// Descriptor for creation of DataBinding
	struct DataBindingDesc
	{
		U32 Location;
		U8 Options = 0;
		std::vector<BindingRange> Ranges;
		std::vector<SamplerDesc> Samplers;

		void AddRange(BindingRange&& range) noexcept { Ranges.emplace_back(std::forward<BindingRange>(range)); }
		void AddSampler(SamplerDesc&& sampler) noexcept { Samplers.emplace_back(std::forward<SamplerDesc>(sampler)); }
		void Append(const DataBindingDesc& binding) noexcept
		{
			Ranges.insert(Ranges.end(), binding.Ranges.begin(), binding.Ranges.end());
			Samplers.insert(Samplers.end(), binding.Samplers.begin(), binding.Samplers.end());
		}
	};
}
#include "GFX/Binding/SchemaDesc.h"

namespace ZE::GFX::Binding
{
	void SchemaDesc::Append(const SchemaDesc& binding, Resource::ShaderTypes useShaders) noexcept
	{
		if (useShaders == Resource::ShaderType::All)
		{
			Ranges.reserve(Ranges.size() + binding.Ranges.size());
			Ranges.insert(Ranges.end(), binding.Ranges.begin(), binding.Ranges.end());
		}
		else
		{
			for (const auto& range : binding.Ranges)
			{
				if (range.Shaders & useShaders)
				{
					Ranges.emplace_back(range);
					Ranges.back().Shaders &= useShaders;
				}
			}
		}
		AppendSamplers(binding);
	}

	void SchemaDesc::AppendSamplers(const SchemaDesc& binding) noexcept
	{
		Samplers.reserve(Samplers.size() + binding.Samplers.size());
		Samplers.insert(Samplers.end(), binding.Samplers.begin(), binding.Samplers.end());
	}
}
#include "GFX/Binding/SchemaDesc.h"

namespace ZE::GFX::Binding
{
	void SchemaDesc::AppendRanges(const std::vector<Range>& ranges, Resource::ShaderTypes useShaders) noexcept
	{
		if (useShaders == Resource::ShaderType::All)
		{
			Ranges.reserve(Ranges.size() + ranges.size());
			Ranges.insert(Ranges.end(), ranges.begin(), ranges.end());
		}
		else
		{
			for (const auto& range : ranges)
			{
				if (range.Shaders & useShaders)
				{
					Ranges.emplace_back(range);
					Ranges.back().Shaders &= useShaders;
				}
			}
		}
	}

	void SchemaDesc::AppendSamplers(const std::vector<Resource::SamplerDesc>& samplers) noexcept
	{
		Samplers.reserve(Samplers.size() + samplers.size());
		Samplers.insert(Samplers.end(), samplers.begin(), samplers.end());
	}
}
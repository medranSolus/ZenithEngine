#pragma once

namespace ZE::Data
{
	// Options for loading external model
	typedef U8 ExternalModelOptions;
	// Single option for loading external model
	enum class ExternalModelOption : ExternalModelOptions
	{
		None = 0,

		// Extract roughness from combined texture with metalness on channel R. Must be used together with same set of metalness flags and not with same channel
		ExtractRoughnessChannelR = 0x01,
		// Extract roughness from combined texture with metalness on channel G. Must be used together with same set of metalness flags and not with same channel, default behavior
		ExtractRoughnessChannelG = 0x02,
		// Extract roughness from combined texture with metalness on channel B. Must be used together with same set of metalness flags and not with same channel
		ExtractRoughnessChannelB = 0x03,
		// Extract roughness from combined texture with metalness on channel A. Must be used together with same set of metalness flags and not with same channel
		ExtractRoughnessChannelA = 0x04,
		// Mask to get all roughness extraction channels
		ExtractRoughnessMask = ExtractRoughnessChannelR | ExtractRoughnessChannelG | ExtractRoughnessChannelB | ExtractRoughnessChannelA,

		// Extract metalness from combined texture with roughness on channel R. Must be used together with same set of roughness flags and not with same channel, default behavior
		ExtractMetalnessChannelR = ExtractRoughnessChannelR << 3,
		// Extract metalness from combined texture with roughness on channel G. Must be used together with same set of roughness flags and not with same channel
		ExtractMetalnessChannelG = ExtractRoughnessChannelG << 3,
		// Extract metalness from combined texture with roughness on channel B. Must be used together with same set of roughness flags and not with same channel
		ExtractMetalnessChannelB = ExtractRoughnessChannelB << 3,
		// Extract metalness from combined texture with roughness on channel A. Must be used together with same set of roughness flags and not with same channel
		ExtractMetalnessChannelA = ExtractRoughnessChannelA << 3,
		// Mask to get all metalness extraction channels
		ExtractMetalnessMask = ExtractMetalnessChannelR | ExtractMetalnessChannelG | ExtractMetalnessChannelB | ExtractMetalnessChannelA,

		// Flip UV coordinates for given model
		FlipUV = 0x40,
	};
	ZE_ENUM_OPERATORS(ExternalModelOption, ExternalModelOptions);
}
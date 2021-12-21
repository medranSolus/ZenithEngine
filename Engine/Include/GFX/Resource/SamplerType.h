#pragma once

namespace ZE::GFX::Resource
{
	// Operation performed by sampler. There are 2 groups to be specified, Operation Type and Sampling Type.
	// Sampling type can be specified using Anisotropic, Point or Linear filtering.
	// Additionally every sampling type (Minification, Magnification, Mip-level sampling) can be separately specified
	// as either Point or Linear (where Point is default option). For example, to use Point sampling for Magnification
	// and Linear for Minification and Mip-level use:
	// LinearMinification | LinearMipSampling
	// Operation type specifies what type of operation will be performed after texture fetch.
	// It can be independently combined with sampling type, and default operation is filtering. All operations are exclusive
	enum SamplerType : U8
	{
		// Equal to Point Filtering
		Default = 0b00,
		// Only for identyfication of operation flags
		OperationType = 0b11,
		Filter        = 0b00,
		Comparison    = 0b01,
		Minimum       = 0b10,
		Maximum       = 0b11,

		LinearMinification  = 0b000100,
		LinearMagnification = 0b001000,
		LinearMipSampling   = 0b010000,
		Anisotropic         = 0b111100,
		Point               = 0b000000,
		Linear = LinearMipSampling | LinearMagnification | LinearMinification,
	};
}
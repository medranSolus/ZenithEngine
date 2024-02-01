#pragma once
ZE_WARNING_PUSH
#include "XeGTAO.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline
{
	// Data needed by XeGTAO
	struct XeGTAOSettings
	{
		::XeGTAO::GTAOSettings Settings;
		float SliceCount;
		float StepsPerSlice;
	};

	// Data needed by SSSR
	struct SSSRSettings
	{
		float TemporalStabilityFactor = 0.7f;
		float DepthBufferThickness = 0.015f;
		float RoughnessThreshold = 0.2f;
		float VarianceThreshold = 0.0f;
		U32 MaxTraversalIntersections = 128;
		U32 MinTraversalOccupancy = 4;
		U32 MostDetailedMip = 0;
		U32 SamplesPerQuad = 1;
		bool TemporalVarianceGuidedTracingEnabled = true;
	};
}
#include "EngineParams.h"

namespace ZE
{
	void EngineParams::SetupParser(CmdParser& parser) noexcept
	{
		parser.AddNumber("width");
		parser.AddNumber("height");
		parser.AddNumber("desc-pool-size", 10000);
		parser.AddOption("min-pass-dist");
		parser.AddNumber("shadow-map-size", 1024);
		parser.AddOption("single-line-perf-entry");
	}

	void EngineParams::SetParsedParams(const CmdParser& parser, EngineParams& params) noexcept
	{
		params.Width = parser.GetNumber("width");
		params.Height = parser.GetNumber("height");
		params.GraphicsDescriptorPoolSize = parser.GetNumber("desc-pool-size");
		params.SingleLinePerfEntry = parser.GetOption("single-line-perf-entry");
		params.MinimizeRenderPassDistances = parser.GetOption("min-pass-dist");
		if (params.CustomRendererDesc == nullptr)
			params.CoreRendererParams.ShadowMapSize = parser.GetNumber("shadow-map-size");
	}
}
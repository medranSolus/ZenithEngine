#include "EngineParams.h"

namespace ZE
{
	void EngineParams::SetupParser(CmdParser& parser) noexcept
	{
		parser.AddNumber("width");
		parser.AddNumber("height");
		parser.AddNumber("descPoolSize", 10000);
		parser.AddOption("minPassDist");
		parser.AddNumber("shadowMapSize", 1024);
		parser.AddOption("singleLinePerfEntry");
	}

	void EngineParams::SetParsedParams(const CmdParser& parser, EngineParams& params) noexcept
	{
		params.Width = parser.GetNumber("width");
		params.Height = parser.GetNumber("height");
		params.GraphicsDescriptorPoolSize = parser.GetNumber("descPoolSize");
		params.SingleLinePerfEntry = parser.GetOption("singleLinePerfEntry");
		params.MinimizeRenderPassDistances = parser.GetOption("minPassDist");
		if (!params.CustomRenderer)
			params.CoreRendererParams.ShadowMapSize = parser.GetNumber("shadowMapSize");
	}
}
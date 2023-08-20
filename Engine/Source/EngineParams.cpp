#include "EngineParams.h"

namespace ZE
{
	void EngineParams::SetupParser(CmdParser& parser) noexcept
	{
		parser.AddOption("dx11");
		parser.AddOption("dx12");
		parser.AddOption("vulkan");
		parser.AddNumber("backbuffers", 2);
		parser.AddNumber("width");
		parser.AddNumber("height");
		parser.AddNumber("descPoolSize", 10000);
		parser.AddNumber("descScratchCount", 800);
		parser.AddNumber("threadsCount", 0);
		parser.AddOption("minPassDist");
		parser.AddNumber("shadowMapSize", 1024);
		parser.AddOption("singleLinePerfEntry");
	}

	GfxApiType EngineParams::GetParsedApi(const CmdParser& parser, GfxApiType defApi) noexcept
	{
		if (parser.GetOption("dx12"))
			return GfxApiType::DX12;
		if (parser.GetOption("vulkan"))
			return GfxApiType::Vulkan;
		if (parser.GetOption("dx11"))
			return GfxApiType::DX11;
		return defApi;
	}
}
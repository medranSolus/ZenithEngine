#include "SettingsInitParams.h"

namespace ZE
{
	void SettingsInitParams::SetupParser(CmdParser& parser) noexcept
	{
		parser.AddOption("dx11");
		parser.AddOption("dx12");
		parser.AddOption("vulkan");
		parser.AddNumber("backbuffers", 2);
		parser.AddNumber("threadsCount", 0);
		parser.AddOption("pixAttach");
		parser.AddOption("fsr2");
	}

	SettingsInitParams SettingsInitParams::GetParsedParams(const CmdParser& parser, const char* appName, U32 appVersion, U8 staticThreadsCount, GfxApiType defApi) noexcept
	{
		SettingsInitParams params = {};

		params.AppName = appName;
		params.AppVersion = appVersion;
		params.GraphicsAPI = GetParsedApi(parser, defApi);
		params.BackbufferCount = parser.GetNumber("backbuffers");
		params.StaticThreadsCount = staticThreadsCount;
		params.CustomThreadPoolThreadsCount = Utils::SafeCast<U8>(parser.GetNumber("threadsCount"));
		params.AllowPIXAttach = parser.GetOption("pixAttach");
		params.Upscaler = parser.GetOption("fsr2") ? GFX::UpscalerType::Fsr2 : GFX::UpscalerType::None;

		return params;
	}

	GfxApiType SettingsInitParams::GetParsedApi(const CmdParser& parser, GfxApiType defApi) noexcept
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
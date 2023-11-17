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
		parser.AddOption("pix");
		parser.AddOption("fsr2");
		parser.AddOption("fsr1");
		parser.AddOption("xess");
		parser.AddOption("xegtao");
		parser.AddOption("cacao");
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
		params.AllowPIXAttach = parser.GetOption("pix");

		if (parser.GetOption("fsr2"))
			params.Upscaler = GFX::UpscalerType::Fsr2;
		else if (parser.GetOption("xess"))
			params.Upscaler = GFX::UpscalerType::XeSS;
		else if (parser.GetOption("fsr1"))
			params.Upscaler = GFX::UpscalerType::Fsr1;
		else
			params.Upscaler = GFX::UpscalerType::None;

		if (parser.GetOption("xegtao"))
			params.AmbientOcclusion = GFX::AOType::XeGTAO;
		else if (parser.GetOption("cacao"))
			params.AmbientOcclusion = GFX::AOType::CACAO;
		else
			params.AmbientOcclusion = GFX::AOType::None;

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
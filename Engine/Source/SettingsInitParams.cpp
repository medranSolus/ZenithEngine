#include "SettingsInitParams.h"

namespace ZE
{
	void SettingsInitParams::SetupParser(CmdParser& parser) noexcept
	{
		parser.AddOption("dx11");
		parser.AddOption("dx12");
		parser.AddOption("vulkan");
		parser.AddNumber("backbuffers", 2);
		parser.AddNumber("threads-count", 0);
		parser.AddOption("pix");
		parser.AddOption("gpu-validation");
		parser.AddOption("ffx-fsr");
		parser.AddOption("fsr3");
		parser.AddOption("fsr2");
		parser.AddOption("fsr1");
		parser.AddOption("xess");
		parser.AddOption("nis");
		parser.AddOption("dlss");
		parser.AddOption("xegtao");
		parser.AddOption("cacao");
		parser.AddOption("no-async-ao");
		parser.AddOption("sssr");
		parser.AddOption("always-copy-source-gpu-data");
		parser.AddOption("no-culling");
		parser.AddOption("split-render-submits");
		parser.AddOption("ibl");
		parser.AddOption("tonemap-exp");
		parser.AddOption("reinhard");
		parser.AddOption("reinhard-x");
		parser.AddOption("reinhard-luma");
		parser.AddOption("reinhard-jodie");
		parser.AddOption("reinhard-white");
		parser.AddOption("tonemap-rbdh");
		parser.AddOption("filmic-hable");
		parser.AddOption("filmic-vdr");
		parser.AddOption("aces");
		parser.AddOption("aces-nautilus");
		parser.AddOption("agx");
		parser.AddOption("khronos-pbr-neutral");
		parser.AddOption("lpm");
	}

	SettingsInitParams SettingsInitParams::GetParsedParams(const CmdParser& parser, const char* appName, U32 appVersion, U8 staticThreadsCount, GfxApiType defApi) noexcept
	{
		SettingsInitParams params = {};

		params.AppName = appName;
		params.AppVersion = appVersion;
		params.GraphicsAPI = GetParsedApi(parser, defApi);
		params.BackbufferCount = parser.GetNumber("backbuffers");
		params.StaticThreadsCount = staticThreadsCount;
		params.CustomThreadPoolThreadsCount = Utils::SafeCast<U8>(parser.GetNumber("threads-count"));
		if (parser.GetOption("pix"))
			params.Flags |= SettingsInitFlag::AllowPIXAttach;
		if (parser.GetOption("gpu-validation"))
			params.Flags |= SettingsInitFlag::EnableGPUValidation;
		if (parser.GetOption("sssr"))
			params.Flags |= SettingsInitFlag::EnableSSSR;

		if (parser.GetOption("ffx-fsr"))
			params.Upscaler = GFX::UpscalerType::FfxFsr;
		else if (parser.GetOption("fsr3"))
			params.Upscaler = GFX::UpscalerType::Fsr3;
		else if (parser.GetOption("dlss"))
			params.Upscaler = GFX::UpscalerType::DLSS;
		else if (parser.GetOption("xess"))
			params.Upscaler = GFX::UpscalerType::XeSS;
		else if (parser.GetOption("fsr2"))
			params.Upscaler = GFX::UpscalerType::Fsr2;
		else if (parser.GetOption("fsr1"))
			params.Upscaler = GFX::UpscalerType::Fsr1;
		else if (parser.GetOption("nis"))
			params.Upscaler = GFX::UpscalerType::NIS;
		else
			params.Upscaler = GFX::UpscalerType::None;

		if (parser.GetOption("xegtao"))
			params.AmbientOcclusion = GFX::AOType::XeGTAO;
		else if (parser.GetOption("cacao"))
			params.AmbientOcclusion = GFX::AOType::CACAO;
		else
			params.AmbientOcclusion = GFX::AOType::None;

		if (parser.GetOption("tonemap-exp"))
			params.Tonemapper = GFX::TonemapperType::Exposure;
		else if (parser.GetOption("reinhard"))
			params.Tonemapper = GFX::TonemapperType::Reinhard;
		else if (parser.GetOption("reinhard-x"))
			params.Tonemapper = GFX::TonemapperType::ReinhardExtended;
		else if (parser.GetOption("reinhard-luma"))
			params.Tonemapper = GFX::TonemapperType::ReinhardLuma;
		else if (parser.GetOption("reinhard-jodie"))
			params.Tonemapper = GFX::TonemapperType::ReinhardLumaJodie;
		else if (parser.GetOption("reinhard-white"))
			params.Tonemapper = GFX::TonemapperType::ReinhardLumaPreserveWhite;
		else if (parser.GetOption("tonemap-rbdh"))
			params.Tonemapper = GFX::TonemapperType::RomBinDaHouse;
		else if (parser.GetOption("filmic-hable"))
			params.Tonemapper = GFX::TonemapperType::FilmicHable;
		else if (parser.GetOption("filmic-vdr"))
			params.Tonemapper = GFX::TonemapperType::FilmicVDR;
		else if (parser.GetOption("aces"))
			params.Tonemapper = GFX::TonemapperType::ACES;
		else if (parser.GetOption("aces-nautilus"))
			params.Tonemapper = GFX::TonemapperType::ACESNautilus;
		else if (parser.GetOption("agx"))
			params.Tonemapper = GFX::TonemapperType::AgX;
		else if (parser.GetOption("khronos-pbr-neutral"))
			params.Tonemapper = GFX::TonemapperType::KhronosPBRNeutral;
		else if (parser.GetOption("lpm"))
			params.Tonemapper = GFX::TonemapperType::LPM;
		else
			params.Tonemapper = GFX::TonemapperType::None;

		if (!parser.GetOption("no-async-ao"))
			params.Flags |= SettingsInitFlag::AsyncAO;
		if (parser.GetOption("always-copy-source-gpu-data"))
			params.Flags |= SettingsInitFlag::AlwaysCopySourceGPUData;
		if (parser.GetOption("no-culling"))
			params.Flags |= SettingsInitFlag::DisableCulling;
		if (parser.GetOption("split-render-submits"))
			params.Flags |= SettingsInitFlag::SplitRenderSubmissions;
		if (parser.GetOption("ibl"))
			params.Flags |= SettingsInitFlag::EnableIBL;

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
#pragma once

namespace ZE::GFX
{
	// Type of Tonemapper used in the rendering pipeline
	enum class TonemapperType : U8
	{
		None,
		Exposure,
		Reinhard,
		ReinhardExtended,
		ReinhardLuma,
		ReinhardLumaJodie,
		ReinhardLumaPreserveWhite,
		RomBinDaHouse,
		FilmicHable,
		FilmicVDR,
		ACES,
		ACESNautilus,
		AgX,
		KhronosPBRNeutral,
		LPM
	};
}
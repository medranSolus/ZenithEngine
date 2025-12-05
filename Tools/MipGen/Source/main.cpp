#include "GFX/Surface.h"
#include "CmdParser.h"
#include "json.hpp"

namespace json = nlohmann;
using namespace ZE;

enum ResultCode : int
{
	NoWorkPerformed = 1,
	Success = 0,
	NoSourceFile = -1,
	CannotLoadFile = -2,
	CannotSaveFile = -3,
	CannotPerformOperation = -4,
};

struct MipParams
{
	std::string_view Source = "";
	std::string_view OutFile = "";
	U32 Cores = 1;
	bool GammaCorrection = false;
	bool SrcOriginalLayer = false;
	float AlphaTestTreshold = FLT_MAX;
	float FilterCoeffParam = 0.0f;
	U32 WindowSize = 2;
	Math::FilterType Filter = Math::FilterType::Box;
};

struct Sample
{
	union
	{
		S8 SInt8;
		S16 SInt16;
		S32 SInt32;
		U32 UInt;
		float Float;
	} RGBA[4];
};

ResultCode ProcessJsonCommand(const json::json& command) noexcept;
ResultCode RunJob(MipParams& job) noexcept;
Sample GetPixelSample(U8* memory, U8 channelSize, U8 channelCount, bool gammaCorrection) noexcept;
Float4 ConvertToFloat(const Sample& pixel, PixelFormat format, U8 channelCount) noexcept;
Sample ConvertToSourceFormat(const Float4& val, PixelFormat format, U8 channelCount, bool gammaCorrection) noexcept;

int main(int argc, char* argv[])
{
	CmdParser parser;
	parser.AddOption("help-filter");
	parser.AddOption("help-filter-coeff-param");
	parser.AddOption("gamma-correction");
	parser.AddOption("src-org-layer");
	parser.AddOption("box", 'b');
	parser.AddOption("gamma-average", 'g');
	parser.AddOption("bilinear", 'l');
	parser.AddOption("kaiser", 'k');
	parser.AddOption("lanczos", 'z');
	parser.AddOption("gauss", 'u');
	parser.AddNumber("filter", 0, 'f');
	parser.AddNumber("window-size", 2, 'w'); // Bilinear will override this to minimal value 2
	parser.AddNumber("cores", 1, 'c');
	parser.AddFloat("filter-coeff-param");
	parser.AddString("source", "", 's');
	parser.AddString("out", "", 'o');
	parser.AddString("json", "", 'j');
	parser.Parse(argc, argv);

	if (parser.GetOption("help-filter"))
	{
		Logger::InfoNoFile("Mip generation filter help:");
		Logger::InfoNoFile("  0:Box, 1:GammaAverage, 2:Bilinear, 3:Kaiser, 4:Lanczos, 5:Gauss");
		return ResultCode::Success;
	}

	if (parser.GetOption("help-filter-coeff-param"))
	{
		Logger::InfoNoFile("Filter coefficient parameter help:");
		Logger::InfoNoFile("  This is additional parameter used when creating complex filtration windows and it's meaning is depended on algorithm used.");
		Logger::InfoNoFile("  For Kaiser filter this is alpha parameter. (ex. value 7.64)");
		Logger::InfoNoFile("  For Gauss filter this is sigma parameter. (ex. value 2.6)");
		Logger::InfoNoFile("  Other filter types do not utilize this value.");
		return ResultCode::Success;
	}

	std::string_view json = parser.GetString("json");
	if (!json.empty())
	{
		std::ifstream fin(json.data());
		if (!fin.good())
		{
			fin.close();
			Logger::Error("Cannot open JSON batch job file \"" + std::string(json) + "\"!");
		}
		else
		{
			json::json jsonArray;
			fin >> jsonArray;
			ResultCode retCode = ResultCode::Success;
			if (jsonArray.is_array())
			{
				for (const auto& item : jsonArray)
				{
					retCode = ProcessJsonCommand(item);
					if (retCode != ResultCode::Success)
						return retCode;
				}
			}
			else
				retCode = ProcessJsonCommand(jsonArray);
			if (retCode != ResultCode::Success)
				return retCode;
		}
	}

	MipParams params = {};
	params.Source = parser.GetString("source");
	if (params.Source.empty())
	{
		if (!json.empty())
			return ResultCode::Success;
		Logger::Error("No source file specified!");
		return ResultCode::NoSourceFile;
	}

	params.OutFile = parser.GetString("out");
	if (params.OutFile.empty())
		params.OutFile = params.Source;
	params.Cores = parser.GetNumber("cores");
	params.GammaCorrection = parser.GetOption("gamma-correction");
	params.SrcOriginalLayer = parser.GetOption("src-org-layer");
	params.FilterCoeffParam = parser.GetFloat("filter-coeff-param");
	params.WindowSize = parser.GetNumber("window-size");
	params.Filter = static_cast<Math::FilterType>(parser.GetNumber("filter"));

	if (parser.GetOption("box"))
		params.Filter = Math::FilterType::Box;
	else if (parser.GetOption("gamma-average"))
		params.Filter = Math::FilterType::GammaAverage;
	else if (parser.GetOption("bilinear"))
		params.Filter = Math::FilterType::Bilinear;
	else if (parser.GetOption("kaiser"))
		params.Filter = Math::FilterType::Kaiser;
	else if (parser.GetOption("lanczos"))
		params.Filter = Math::FilterType::Lanczos;
	else if (parser.GetOption("gauss"))
		params.Filter = Math::FilterType::Gauss;

	return RunJob(params);
}

ResultCode ProcessJsonCommand(const json::json& command) noexcept
{
	MipParams params = {};
	if (command.contains("source"))
		params.Source = command["source"].get<std::string_view>();
	else
	{
		Logger::Error("JSON command missing required \"source\" parameter!");
		return ResultCode::NoSourceFile;
	}

	if (command.contains("out"))
		params.OutFile = command["out"].get<std::string_view>();
	else
		params.OutFile = params.Source;

	if (command.contains("cores"))
		params.Cores = command["cores"].get<U32>();
	if (command.contains("gamma-correction"))
		params.GammaCorrection = command["gamma-correction"].get<bool>();
	if (command.contains("src-org-layer"))
		params.SrcOriginalLayer = command["src-org-layer"].get<bool>();
	if (command.contains("window-size"))
		params.FilterCoeffParam = command["filter-coeff-param"].get<float>();
	if (command.contains("window-size"))
		params.WindowSize = command["window-size"].get<U32>();
	if (command.contains("filter"))
		params.Filter = static_cast<Math::FilterType>(command["filter"].get<U32>());

	return RunJob(params);
}

ResultCode RunJob(MipParams& job) noexcept
{
	if (job.WindowSize < 2)
	{
		job.WindowSize = 2;
		Logger::Warning("Window size cannot be less than 2, overriding specified value.");
	}
	else if (job.WindowSize != 2 && job.Filter == Math::FilterType::Bilinear)
	{
		job.WindowSize = 2;
		Logger::Warning("Bilinear filter always uses window size of 2, overriding specified value.");
	}

	GFX::Surface surface;
	if (!surface.Load(job.Source, false, true))
	{
		Logger::Error("Cannot load file \"" + std::string(job.Source) + "\"!");
		return ResultCode::CannotLoadFile;
	}

	// Create filter coefficients if needed
	std::vector<float> filterCoeffs;
	if (job.Filter != Math::FilterType::Box && job.Filter != Math::FilterType::GammaAverage && job.Filter != Math::FilterType::Bilinear)
	{
		filterCoeffs.resize((job.WindowSize >> 1) + (job.WindowSize & 1));
		const U32 coeffSize = Utils::SafeCast<U32>(filterCoeffs.size());

		float firstCoeff = 0.0f;
		float coeffSum = 0.0f;
		switch (job.Filter)
		{
		case Math::FilterType::Kaiser:
		{
			if (job.FilterCoeffParam == 0.0f)
				job.FilterCoeffParam = 7.64f;

			for (U32 i = 0; i < coeffSize; ++i)
			{
				// Regular Kaiser window reaches 1 at length / 2 so need to scale it correctly
				const float k = Math::Kaiser(static_cast<float>(i + coeffSize), job.FilterCoeffParam, static_cast<float>(coeffSize * 2));
				filterCoeffs.at(i) = k;
				coeffSum += k;
			}
			break;
		}
		case Math::FilterType::Lanczos:
		{
			for (U32 i = 0; i < coeffSize; ++i)
			{
				const float l = Math::Lanczos(static_cast<float>(i), static_cast<float>(coeffSize));
				filterCoeffs.at(i) = l;
				coeffSum += l;
			}
			break;
		}
		case Math::FilterType::Gauss:
		{
			if (job.FilterCoeffParam == 0.0f)
				job.FilterCoeffParam = 2.6f;

			for (U32 i = 0; i < coeffSize; ++i)
			{
				const float g = Math::Gauss(static_cast<float>(i), job.FilterCoeffParam);
				filterCoeffs.at(i) = g;
				coeffSum += g;
			}
			break;
		}
		default:
			break;
		}

		// Normalize filter coefficients, symetric kernel requires doubling the sum
		// but just for even windows, odd windows have center coeff counted once
		coeffSum *= 2.0f;
		if (job.WindowSize & 1)
			coeffSum -= filterCoeffs.front();

		for (float& coeff : filterCoeffs)
			coeff /= coeffSum;
	}

	// TODO: Multithreaded version
	const U8 channelCount = Utils::GetChannelCount(surface.GetFormat());
	const PixelFormat format = Utils::GetSingleChannelFormat(surface.GetFormat());
	const U8 pixelSize = surface.GetPixelSize();
	const U8 channelSize = pixelSize / channelCount;
	const bool alphaRemap = channelCount == 4 && job.AlphaTestTreshold != FLT_MAX;
	const S32 halfWindow = Utils::SafeCast<S32>(job.WindowSize) >> 1;
	const U64 firstSliceSize = surface.GetSliceByteSize();

	U8* srcBuffer = surface.GetBuffer();
	U8* mipGenBuffer = srcBuffer + firstSliceSize;
	for (U16 a = 0; a < surface.GetArraySize(); ++a)
	{
		U32 srcWidth = surface.GetWidth();
		U32 srcHeight = surface.GetHeight();
		U16 srcDepth = surface.GetDepth();
		U16 srcMip = 0;

		U32 mipWidth = srcWidth >> 1;
		U32 mipHeight = srcHeight >> 1;
		U16 mipDepth = srcDepth >> 1;

		if (job.SrcOriginalLayer)
			srcBuffer = surface.GetImage(a, 0, 0);

		for (U16 mip = 1; mip < surface.GetMipCount(); ++mip)
		{
			if (mipWidth == 0)
				mipWidth = 1;
			if (mipHeight == 0)
				mipHeight = 1;
			if (mipDepth == 0)
				mipDepth = 1;

			const U64 srcSliceSize = surface.GetSliceByteSize(srcMip);
			const U64 srcRowSize = surface.GetRowByteSize(srcMip);
			const U64 sliceSize = surface.GetSliceByteSize(mip);
			const U64 rowSize = surface.GetRowByteSize(mip);
			const U32 mipDiff = 1 << (mip - srcMip);
			for (U16 d = 0; d < mipDepth; ++d)
			{
				for (U32 y = 0; y < mipHeight; ++y)
				{
					// Generate row sampling points
					std::vector<U32> rowOffsets;
					rowOffsets.reserve(job.WindowSize);

					const S32 baseY = Utils::SafeCast<S32>(y * mipDiff) + 1;
					for (S32 i = -halfWindow - (job.WindowSize & 1); i < halfWindow; ++i)
						rowOffsets.emplace_back(Math::MirrorCoord(Utils::SafeCast<S32>(baseY) + i, Utils::SafeCast<S32>(srcHeight)));

					const U64 offset = rowSize * y;
					for (U32 x = 0; x < mipWidth; ++x)
					{
						// Generate column sampling points
						std::vector<U32> columnOffsets;
						columnOffsets.reserve(job.WindowSize);

						const S32 baseX = Utils::SafeCast<S32>(x * mipDiff) + 1;
						for (S32 i = -halfWindow - (job.WindowSize & 1); i < halfWindow; ++i)
							columnOffsets.emplace_back(Math::MirrorCoord(Utils::SafeCast<S32>(baseX) + i, Utils::SafeCast<S32>(srcWidth)));

						// Generate samples inside window
						std::vector<Float4> samples;
						samples.reserve(static_cast<U64>(job.WindowSize) * job.WindowSize);
						for (U64 rowOffset : rowOffsets)
						{
							for (U32 colOffset : columnOffsets)
							{
								Sample sample = GetPixelSample(srcBuffer + colOffset * pixelSize + rowOffset * srcRowSize, channelSize, channelCount, job.GammaCorrection);
								// Convert to float for processing
								samples.emplace_back(ConvertToFloat(sample, format, channelCount));
							}
						}

						Float4 mipVal = {};
						Math::XMStoreFloat4(&mipVal, Math::ApplyFilter(job.Filter, samples, 0.5f, 0.5f, &filterCoeffs));

						// https://asawicki.info/articles/alpha_test.php5
						if (alphaRemap)
							mipVal.w = std::max(mipVal.w, (mipVal.w + 2.0f * job.AlphaTestTreshold) / 3.0f);

						// Convert back to original format
						Sample pixel = ConvertToSourceFormat(mipVal, format, channelCount, job.GammaCorrection);
						for (U8 i = 0; i < channelCount; ++i)
							std::memcpy(mipGenBuffer + static_cast<U64>(x) * pixelSize + offset + i * channelSize, &pixel.RGBA[i].UInt, channelSize);
					}
				}

				srcBuffer += srcSliceSize;
				mipGenBuffer += sliceSize;
			}

			if (!job.SrcOriginalLayer)
			{
				srcWidth >>= 1;
				if (srcWidth == 0)
					srcWidth = 1;
				srcHeight >>= 1;
				if (srcHeight == 0)
					srcHeight = 1;
				srcDepth >>= 1;
				if (srcDepth == 0)
					srcDepth = 1;
				++srcMip;
			}
			else
				srcBuffer -= firstSliceSize * srcDepth;

			mipWidth >>= 1;
			mipHeight >>= 1;
			mipDepth >>= 1;
		}
		if (!job.SrcOriginalLayer)
			srcBuffer += surface.GetSliceByteSize(surface.GetMipCount() - 1);
		mipGenBuffer += firstSliceSize;
	}

	if (surface.Save(job.OutFile))
	{
		Logger::Info("Saved texture to file \"" + std::string(job.OutFile) + "\"");
		return ResultCode::Success;
	}
	Logger::Error("Error saving to \"" + std::string(job.OutFile) + "\"!");
	return ResultCode::CannotSaveFile;
}

Sample GetPixelSample(U8* memory, U8 channelSize, U8 channelCount, bool gammaCorrection) noexcept
{
	Sample pixelValue = {};
	pixelValue.RGBA[0].UInt = 0;
	pixelValue.RGBA[1].UInt = 0;
	pixelValue.RGBA[2].UInt = 0;
	pixelValue.RGBA[3].UInt = 0;

	for (U8 i = 0; i < channelCount; ++i)
	{
		for (U8 j = 0; j < channelSize; ++j)
			pixelValue.RGBA[i].UInt |= static_cast<U32>(memory[j]) << (j * 8);
		memory += channelSize;
	}
	return pixelValue;
}

Float4 ConvertToFloat(const Sample& pixel, PixelFormat format, U8 channelCount) noexcept
{
	Sample val = {};
	switch (format)
	{
	default:
		ZE_ENUM_UNHANDLED();
	case PixelFormat::Unknown:
		ZE_FAIL("Unsupported pixel format for float convertion!");
		[[fallthrough]];
	case PixelFormat::R32_Float:
	{
		std::memcpy(&val, &pixel, sizeof(Sample));
		break;
	}
	case PixelFormat::R32_UInt:
	case PixelFormat::R16_UInt:
	case PixelFormat::R8_UInt:
	{
		for (U8 i = 0; i < channelCount; ++i)
			val.RGBA[i].Float = static_cast<float>(pixel.RGBA[i].UInt);
		break;
	}
	case PixelFormat::R32_SInt:
	{
		for (U8 i = 0; i < channelCount; ++i)
			val.RGBA[i].Float = static_cast<float>(pixel.RGBA[i].SInt32);
		break;
	}
	case PixelFormat::R16_SInt:
	{
		for (U8 i = 0; i < channelCount; ++i)
			val.RGBA[i].Float = static_cast<float>(pixel.RGBA[i].SInt16);
		break;
	}
	case PixelFormat::R8_SInt:
	{
		for (U8 i = 0; i < channelCount; ++i)
			val.RGBA[i].Float = static_cast<float>(pixel.RGBA[i].SInt8);
		break;
	}
	case PixelFormat::R16_UNorm:
	{
		for (U8 i = 0; i < channelCount; ++i)
			val.RGBA[i].Float = static_cast<float>(pixel.RGBA[i].UInt) / UINT16_MAX;
		break;
	}
	case PixelFormat::R8_UNorm:
	{
		for (U8 i = 0; i < channelCount; ++i)
			val.RGBA[i].Float = static_cast<float>(pixel.RGBA[i].UInt) / UINT8_MAX;
		break;
	}
	case PixelFormat::R16_SNorm:
	{
		for (U8 i = 0; i < channelCount; ++i)
			val.RGBA[i].Float = static_cast<float>(pixel.RGBA[i].SInt16) / INT16_MAX;
		break;
	}
	case PixelFormat::R8_SNorm:
	{
		for (U8 i = 0; i < channelCount; ++i)
			val.RGBA[i].Float = static_cast<float>(pixel.RGBA[i].UInt) / INT8_MAX;
		break;
	}
	case PixelFormat::R16_Float:
	{
		for (U8 i = 0; i < channelCount; ++i)
			val.RGBA[i].Float = Math::FP16::DecodeFloat16(static_cast<U16>(pixel.RGBA[i].UInt));
		break;
	}
	}
	return { val.RGBA[0].Float, val.RGBA[1].Float, val.RGBA[2].Float, val.RGBA[3].Float };
}

Sample ConvertToSourceFormat(const Float4& val, PixelFormat format, U8 channelCount, bool gammaCorrection) noexcept
{
	Sample pixel = {};
	pixel.RGBA[0].Float = val.x;
	pixel.RGBA[1].Float = val.y;
	pixel.RGBA[2].Float = val.z;
	pixel.RGBA[3].Float = val.w;
	switch (format)
	{
	default:
		ZE_ENUM_UNHANDLED();
	case PixelFormat::Unknown:
		ZE_FAIL("Unsupported pixel format for converting from float!");
		[[fallthrough]];
	case PixelFormat::R32_Float:
		break;
	case PixelFormat::R32_UInt:
	{
		for (U8 i = 0; i < channelCount; ++i)
			pixel.RGBA[i].UInt = static_cast<U32>(std::clamp(static_cast<U64>(pixel.RGBA[i].Float), 0ULL, static_cast<U64>(UINT32_MAX)));
		break;
	}
	case PixelFormat::R16_UInt:
	{
		for (U8 i = 0; i < channelCount; ++i)
			pixel.RGBA[i].UInt = static_cast<U32>(std::clamp(static_cast<U64>(pixel.RGBA[i].Float), 0ULL, static_cast<U64>(UINT16_MAX)));
		break;
	}
	case PixelFormat::R8_UInt:
	{
		for (U8 i = 0; i < channelCount; ++i)
			pixel.RGBA[i].UInt = static_cast<U32>(std::clamp(static_cast<U64>(pixel.RGBA[i].Float), 0ULL, static_cast<U64>(UINT8_MAX)));
		break;
	}
	case PixelFormat::R32_SInt:
	{
		for (U8 i = 0; i < channelCount; ++i)
			pixel.RGBA[i].SInt32 = static_cast<S32>(std::clamp(static_cast<S64>(pixel.RGBA[i].Float), static_cast<S64>(INT32_MIN), static_cast<S64>(INT32_MAX)));
		break;
	}
	case PixelFormat::R16_SInt:
	{
		for (U8 i = 0; i < channelCount; ++i)
			pixel.RGBA[i].SInt16 = static_cast<S16>(std::clamp(static_cast<S64>(pixel.RGBA[i].Float), static_cast<S64>(INT16_MIN), static_cast<S64>(INT16_MAX)));
		break;
	}
	case PixelFormat::R8_SInt:
	{
		for (U8 i = 0; i < channelCount; ++i)
			pixel.RGBA[i].SInt8 = static_cast<S8>(std::clamp(static_cast<S64>(pixel.RGBA[i].Float), static_cast<S64>(INT8_MIN), static_cast<S64>(INT8_MAX)));
		break;
	}
	case PixelFormat::R16_UNorm:
	{
		for (U8 i = 0; i < channelCount; ++i)
			pixel.RGBA[i].UInt = static_cast<U32>(pixel.RGBA[i].Float * UINT16_MAX);
		break;
	}
	case PixelFormat::R8_UNorm:
	{
		for (U8 i = 0; i < channelCount; ++i)
			pixel.RGBA[i].UInt = static_cast<U32>(pixel.RGBA[i].Float * UINT8_MAX);
		break;
	}
	case PixelFormat::R16_SNorm:
	{
		for (U8 i = 0; i < channelCount; ++i)
			pixel.RGBA[i].SInt16 = static_cast<S16>(pixel.RGBA[i].Float * INT16_MAX);
		break;
	}
	case PixelFormat::R8_SNorm:
	{
		for (U8 i = 0; i < channelCount; ++i)
			pixel.RGBA[i].SInt8 = static_cast<S8>(pixel.RGBA[i].Float * INT8_MAX);
		break;
	}
	case PixelFormat::R16_Float:
	{
		for (U8 i = 0; i < channelCount; ++i)
			pixel.RGBA[i].UInt = Math::FP16::EncodeFloat16(pixel.RGBA[i].Float);
		break;
	}
	}
	return pixel;
}
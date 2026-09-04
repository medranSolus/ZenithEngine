#include "CmdParser.h"
#include "TexOps.h"
#include "json.hpp"

namespace json = nlohmann;

enum ResultCode : int
{
	NoWorkPerformed = 1,
	Success = 0,
	NoSourceFile = -1,
	CannotLoadFile = -2,
	CannotSaveFile = -3,
	CannotPerformOperation = -4,
};

struct JobParams
{
	std::string_view Source = "";
	std::string_view OutFile = "";
	U32 Cores = 1;
	bool NoAlpha = false;
	bool FlipY = false;
	bool HdriCubemap = false;
	bool Fp16 = false;
	float FilterCoeffParam = 0.0f;
	Math::FilterType Filter = Math::FilterType::Box;
	U32 WindowSize = 2;
	U32 CubeWidth = 0;
};

ResultCode ProcessJsonCommand(const json::json& command) noexcept;
ResultCode RunJob(const JobParams& job) noexcept;

int main(int argc, char* argv[])
{
	CmdParser parser;
	parser.AddOption("help-cube-filter");
	parser.AddOption("help-filter-coeff-param");
	parser.AddOption("no-alpha", 'a');
	parser.AddOption("flip-y", 'y');
	parser.AddOption("hdri-cubemap", 'q');
	parser.AddOption("fp16", 'f');
	parser.AddFloat("filter-coeff-param");
	parser.AddNumber("cube-filter");
	parser.AddNumber("cube-width");
	parser.AddNumber("window-size", 2);
	parser.AddNumber("cores", 1, 'c');
	parser.AddString("source", "", 's');
	parser.AddString("out", "", 'o');
	parser.AddString("json", "", 'j');
	parser.AddString("log-dir");
	parser.AddString("log-file");
	parser.Parse(argc, argv);

	if (parser.GetOption("help-cube-filter"))
	{
		Logger::InfoNoFile("HDRI cubemap conversion filter help:");
		Logger::InfoNoFile("  0:Box, 1:GammaAverage, 2:Bilinear, 3:Kaiser, 4:Lanczos, 5:Gauss, 6:BicubicSharp, 7:BicubicSmooth");
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

	std::string_view logDir = parser.GetString("log-dir");
	std::string_view logFile = parser.GetString("log-file");
	Logger::SetLogsOuput(logDir.empty() ? Logger::GetDir() : logDir, logFile.empty() ? "log_TexEdit.txt" : logFile);

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

	JobParams params = {};
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
	params.NoAlpha = parser.GetOption("no-alpha");
	params.FlipY = parser.GetOption("flip-y");
	params.HdriCubemap = parser.GetOption("hdri-cubemap");
	params.Fp16 = parser.GetOption("fp16");
	params.FilterCoeffParam = parser.GetFloat("filter-coeff-param");
	params.Filter = static_cast<Math::FilterType>(parser.GetNumber("cube-filter"));
	params.WindowSize = parser.GetNumber("window-size");
	params.CubeWidth = parser.GetNumber("cube-width");

	return RunJob(params);
}

ResultCode ProcessJsonCommand(const json::json& command) noexcept
{
	JobParams params = {};
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
	if (command.contains("no-alpha"))
		params.NoAlpha = command["no-alpha"].get<bool>();
	if (command.contains("flip-y"))
		params.FlipY = command["flip-y"].get<bool>();
	if (command.contains("hdri-cubemap"))
		params.HdriCubemap = command["hdri-cubemap"].get<bool>();
	if (command.contains("fp16"))
		params.Fp16 = command["fp16"].get<bool>();
	if (command.contains("filter-coeff-param"))
		params.FilterCoeffParam = command["filter-coeff-param"].get<float>();
	if (command.contains("cube-filter"))
		params.Filter = static_cast<Math::FilterType>(command["cube-filter"].get<U32>());
	if (command.contains("window-size"))
		params.WindowSize = command["window-size"].get<U32>();
	if (command.contains("cube-width"))
		params.CubeWidth = command["cube-width"].get<U32>();

	return RunJob(params);
}

ResultCode RunJob(const JobParams& job) noexcept
{
	// Early out if nothing to do
	if (!job.NoAlpha && !job.FlipY && !job.HdriCubemap)
	{
		ResultCode retCode = ResultCode::NoWorkPerformed;
		if (job.OutFile == job.Source)
			Logger::Warning("Nothing to do for file \"" + std::string(job.Source) + "\"");
		else
		{
			GFX::Surface surface;
			if (surface.Load(job.Source))
			{
				Logger::Warning("Nothing to do, saving to file \"" + std::string(job.OutFile) + "\"");
				if (!surface.Save(job.OutFile))
				{
					Logger::Error("Error saving to \"" + std::string(job.OutFile) + "\"!");
					retCode = ResultCode::CannotSaveFile;
				}
			}
			else
			{
				Logger::Error("Cannot load file \"" + std::string(job.Source) + "\"!");
				retCode = ResultCode::CannotLoadFile;
			}
		}
		return retCode;
	}

	GFX::Surface surface;
	if (!surface.Load(job.Source))
	{
		Logger::Error("Cannot load file \"" + std::string(job.Source) + "\"!");
		return ResultCode::CannotLoadFile;
	}

	bool saved = false;
	if (job.HdriCubemap)
	{
		if (job.FlipY)
			Logger::Warning("Flipping Y channel while processing HDRI images is not permitted!");
		if (job.WindowSize == 0)
			Logger::Warning("Window size of 0 is not valid for HDRI to cubemap conversion, using default value of 2!");
		if (2 * surface.GetHeight() != surface.GetWidth())
			Logger::Warning("Source image is not in expected 2:1 aspect ratio for HDRI to cubemap conversion!");
		
		U32 width = job.CubeWidth == 0 ? surface.GetHeight() : job.CubeWidth;
		GFX::Surface cubemap(width, width, 1, 1, 6, job.Fp16 ? PixelFormat::R16G16B16A16_Float : (job.NoAlpha ? PixelFormat::R32G32B32_Float : PixelFormat::R32G32B32A32_Float), false);

		TexOps::ConvertToCubemap(surface, cubemap, job.Cores, job.Filter, job.FilterCoeffParam, job.WindowSize == 0 ? 2 : job.WindowSize, job.NoAlpha, job.Fp16);
		Logger::Info("Converted to 6-faced cubemap");
		saved = cubemap.Save(job.OutFile);
	}
	else
	{
		U8 requiredChannels = 0;
		if (job.NoAlpha)
			requiredChannels = 4;
		if (job.FlipY)
			requiredChannels = 2;

		const U8 channelCount = Utils::GetChannelCount(surface.GetFormat());
		if (channelCount < requiredChannels)
		{
			Logger::Error("Source file \"" + std::string(job.Source) + "\" does not have required channels!");
			return ResultCode::CannotPerformOperation;
		}

		TexOps::SimpleProcess(surface, job.Cores, job.NoAlpha, job.FlipY);

		if (job.NoAlpha)
			Logger::Info("Alpha channel reseted.");
		if (job.FlipY)
			Logger::Info("Y channel flipped.");
		saved = surface.Save(job.OutFile);
	}

	if (saved)
	{
		Logger::Info("Saved texture to file \"" + std::string(job.OutFile) + "\"");
		return ResultCode::Success;
	}
	Logger::Error("Error saving to \"" + std::string(job.OutFile) + "\"!");
	return ResultCode::CannotSaveFile;
}
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
	bool Bilinear = false;
};

ResultCode ProcessJsonCommand(const json::json& command) noexcept;
ResultCode RunJob(const JobParams& job) noexcept;

int main(int argc, char* argv[])
{
	CmdParser parser;
	parser.AddOption("no-alpha", 'a');
	parser.AddOption("flip-y", 'y');
	parser.AddOption("hdri-cubemap", 'q');
	parser.AddOption("fp16", 'f');
	parser.AddOption("bilinear", 'b');
	parser.AddNumber("cores", 1, 'c');
	parser.AddString("source", "", 's');
	parser.AddString("out", "", 'o');
	parser.AddString("json", "", 'j');
	parser.Parse(argc, argv);

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
	params.Bilinear = parser.GetOption("bilinear");

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
	if (command.contains("bilinear"))
		params.Bilinear = command["bilinear"].get<bool>();

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
		if (job.NoAlpha || job.FlipY)
			Logger::Warning("Other operations specified during HDRi format processing will be ignored!");
		if (2 * surface.GetHeight() != surface.GetWidth())
			Logger::Warning("Source image is not in expected 2:1 aspect ratio for HDRi to cubemap conversion!");

		GFX::Surface cubemap(surface.GetWidth() / 2, surface.GetHeight(), 1, 1, 6, job.Fp16 ? PixelFormat::R16G16B16A16_Float : PixelFormat::R32G32B32_Float, false);

		TexOps::ConvertToCubemap(surface, cubemap, job.Cores, job.Bilinear, job.Fp16);
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
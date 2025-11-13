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

ResultCode ProcessJsonCommand(const json::json& command) noexcept;
ResultCode RunJob(std::string_view source, std::string_view outFile, U32 cores, bool noAlpha, bool flipY, bool hdriCubemap, bool fp16, bool bilinear) noexcept;

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

	std::string_view source = parser.GetString("source");
	if (source.empty())
	{
		if (!json.empty())
			return ResultCode::Success;
		Logger::Error("No source file specified!");
		return ResultCode::NoSourceFile;
	}

	std::string_view outFile = parser.GetString("out");
	if (outFile.empty())
		outFile = source;
	U32 cores = parser.GetNumber("cores");
	bool noAlpha = parser.GetOption("no-alpha");
	bool flipY = parser.GetOption("flip-y");
	bool hdriCubemap = parser.GetOption("hdri-cubemap");
	bool fp16 = parser.GetOption("fp16");
	bool bilinear = parser.GetOption("bilinear");

	return RunJob(source, outFile, cores, noAlpha, flipY, hdriCubemap, fp16, bilinear);
}

ResultCode ProcessJsonCommand(const json::json& command) noexcept
{
	std::string_view source = "";
	if (command.contains("source"))
		source = command["source"].get<std::string_view>();
	else
	{
		Logger::Error("JSON command missing required \"source\" parameter!");
		return ResultCode::NoSourceFile;
	}

	std::string_view outFile = "";
	if (command.contains("out"))
		outFile = command["out"].get<std::string_view>();
	else
		outFile = source;

	U32 cores = 1;
	if (command.contains("cores"))
		cores = command["cores"].get<U32>();
	bool noAlpha = false;
	if (command.contains("no-alpha"))
		noAlpha = command["no-alpha"].get<bool>();
	bool flipY = false;
	if (command.contains("flip-y"))
		flipY = command["flip-y"].get<bool>();
	bool hdriCubemap = false;
	if (command.contains("hdri-cubemap"))
		hdriCubemap = command["hdri-cubemap"].get<bool>();
	bool fp16 = false;
	if (command.contains("fp16"))
		fp16 = command["fp16"].get<bool>();
	bool bilinear = false;
	if (command.contains("bilinear"))
		bilinear = command["bilinear"].get<bool>();

	return RunJob(source, outFile, cores, noAlpha, flipY, hdriCubemap, fp16, bilinear);
}

ResultCode RunJob(std::string_view source, std::string_view outFile, U32 cores, bool noAlpha, bool flipY, bool hdriCubemap, bool fp16, bool bilinear) noexcept
{
	// Early out if nothing to do
	if (!noAlpha && !flipY && !hdriCubemap)
	{
		ResultCode retCode = ResultCode::NoWorkPerformed;
		if (outFile == source)
			Logger::Warning("Nothing to do for file \"" + std::string(source) + "\"");
		else
		{
			GFX::Surface surface;
			if (surface.Load(source))
			{
				Logger::Warning("Nothing to do, saving to file \"" + std::string(outFile) + "\"");
				if (!surface.Save(outFile))
				{
					Logger::Error("Error saving to \"" + std::string(outFile) + "\"!");
					retCode = ResultCode::CannotSaveFile;
				}
			}
			else
			{
				Logger::Error("Cannot load file \"" + std::string(source) + "\"!");
				retCode = ResultCode::CannotLoadFile;
			}
		}
		return retCode;
	}

	GFX::Surface surface;
	if (!surface.Load(source))
	{
		Logger::Error("Cannot load file \"" + std::string(source) + "\"!");
		return ResultCode::CannotLoadFile;
	}

	bool saved = false;
	if (hdriCubemap)
	{
		if (noAlpha || flipY)
			Logger::Warning("Other operations specified during HDRi format processing will be ignored!");
		GFX::Surface cubemap(surface.GetWidth() / 2, surface.GetHeight(), 1, 1, 6, fp16 ? PixelFormat::R16G16B16A16_Float : PixelFormat::R32G32B32_Float, false);

		TexOps::ConvertToCubemap(surface, cubemap, cores, bilinear, fp16);
		Logger::Info("Converted to 6-faced cubemap");
		saved = cubemap.Save(outFile);
	}
	else
	{
		U8 requiredChannels = 0;
		if (noAlpha)
			requiredChannels = 4;
		if (flipY)
			requiredChannels = 2;

		const U8 channelCount = Utils::GetChannelCount(surface.GetFormat());
		if (channelCount < requiredChannels)
		{
			Logger::Error("Source file \"" + std::string(source) + "\" does not have required channels!");
			return ResultCode::CannotPerformOperation;
		}

		TexOps::SimpleProcess(surface, cores, noAlpha, flipY);

		if (noAlpha)
			Logger::Info("Alpha channel reseted");
		if (flipY)
			Logger::Info("Y channel flipped");
		saved = surface.Save(outFile);
	}

	if (saved)
	{
		Logger::Info("Saved texture to file \"" + std::string(outFile) + "\"");
		return ResultCode::Success;
	}
	Logger::Error("Error saving to \"" + std::string(outFile) + "\"!");
	return ResultCode::CannotSaveFile;
}
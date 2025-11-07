#include "GFX/Surface.h"
#include "CmdParser.h"
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
ResultCode RunJob(std::string_view source, std::string_view outFile, bool noAlpha, bool flipY) noexcept;

int main(int argc, char* argv[])
{
	ZE::CmdParser parser;
	parser.AddOption("no-alpha", 'a');
	parser.AddOption("flip-y", 'y');
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
			ZE::Logger::Error("Cannot open JSON batch job file \"" + std::string(json) + "\"!");
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
		ZE::Logger::Error("No source file specified!");
		return ResultCode::NoSourceFile;
	}

	std::string_view outFile = parser.GetString("out");
	if (outFile.empty())
		outFile = source;
	bool noAlpha = parser.GetOption("no-alpha");
	bool flipY = parser.GetOption("flip-y");

	return RunJob(source, outFile, noAlpha, flipY);
}

ResultCode ProcessJsonCommand(const json::json& command) noexcept
{
	std::string_view source = "";
	if (command.contains("source"))
		source = command["source"].get<std::string_view>();
	else
	{
		ZE::Logger::Error("JSON command missing required \"source\" parameter!");
		return ResultCode::NoSourceFile;
	}

	std::string_view outFile = "";
	if (command.contains("out"))
		outFile = command["out"].get<std::string_view>();
	else
		outFile = source;

	bool noAlpha = false;
	if (command.contains("no-alpha"))
		noAlpha = command["no-alpha"].get<bool>();
	bool flipY = false;
	if (command.contains("flip-y"))
		flipY = command["flip-y"].get<bool>();

	return RunJob(source, outFile, noAlpha, flipY);
}

ResultCode RunJob(std::string_view source, std::string_view outFile, bool noAlpha, bool flipY) noexcept
{
	// Early out if nothing to do
	if (!noAlpha && !flipY)
	{
		ResultCode retCode = ResultCode::NoWorkPerformed;
		if (outFile == source)
			ZE::Logger::Warning("Nothing to do for file \"" + std::string(source) + "\"");
		else
		{
			ZE::GFX::Surface surface;
			if (surface.Load(source))
			{
				ZE::Logger::Warning("Nothing to do, saving to file \"" + std::string(outFile) + "\"");
				if (!surface.Save(outFile))
				{
					ZE::Logger::Error("Error saving to \"" + std::string(outFile) + "\"!");
					retCode = ResultCode::CannotSaveFile;
				}
			}
			else
			{
				ZE::Logger::Error("Cannot load file \"" + std::string(source) + "\"!");
				retCode = ResultCode::CannotLoadFile;
			}
		}
		return retCode;
	}

	U8 requiredChannels = 0;
	if (noAlpha)
		requiredChannels = 4;
	if (flipY)
		requiredChannels = 2;

	ResultCode retCode = ResultCode::Success;
	ZE::GFX::Surface surface;
	if (surface.Load(source))
	{
		const U8 channelCount = ZE::Utils::GetChannelCount(surface.GetFormat());
		if (channelCount < requiredChannels)
		{
			ZE::Logger::Error("Source file \"" + std::string(source) + "\" does not have required channels!");
			retCode = ResultCode::CannotPerformOperation;
		}
		else
		{
			const U8 channelSize = ZE::Utils::GetChannelSize(surface.GetFormat());
			const U8 pixelSize = surface.GetPixelSize();
			const U32 rowSize = surface.GetRowByteSize();
			U8* buffer = surface.GetBuffer();
			for (U16 a = 0; a < surface.GetArraySize(); ++a)
			{
				U32 currentWidth = surface.GetWidth();
				U32 currentHeight = surface.GetHeight();
				U16 currentDepth = surface.GetDepth();
				for (U16 mip = 0; mip < surface.GetMipCount(); ++mip)
				{
					for (U16 d = 0; d < currentDepth; ++d)
					{
						for (U32 y = 0; y < currentHeight; ++y)
						{
							// Only place with row alignment, everything above have slive alignment
							for (U32 x = 0; x < currentWidth; ++x)
							{
								// Assume R8_UNorm for single channel for now
								if (noAlpha)
									buffer[x * pixelSize + channelSize * 3] = 255;
								if (flipY)
									buffer[x * pixelSize + channelSize] = 255 - buffer[channelSize];
							}
							buffer += rowSize;
						}
					}

					currentWidth >>= 1;
					if (currentWidth == 0)
						currentWidth = 1;
					currentHeight >>= 1;
					if (currentHeight == 0)
						currentHeight = 1;
					currentDepth >>= 1;
					if (currentDepth == 0)
						currentDepth = 1;
				}
			}

			if (surface.Save(outFile))
			{
				if (noAlpha)
					ZE::Logger::Info("Alpha channel reseted");
				if (flipY)
					ZE::Logger::Info("Y channel flipped");
				ZE::Logger::Info("Saved texture to file \"" + std::string(outFile) + "\"");
			}
			else
			{
				ZE::Logger::Error("Error saving to \"" + std::string(outFile) + "\"!");
				retCode = ResultCode::CannotSaveFile;
			}
		}
	}
	else
	{
		ZE::Logger::Error("Cannot load file \"" + std::string(source) + "\"!");
		retCode = ResultCode::CannotLoadFile;
	}
	return retCode;
}
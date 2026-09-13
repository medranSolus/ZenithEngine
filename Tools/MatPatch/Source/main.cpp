#include "CmdParser.h"
#include "json.hpp"

namespace json = nlohmann;
using namespace ZE;

enum ResultCode : int
{
	Success = 0,
	NoSourceFile = -1,
	NoMipgenScript = -2,
	CopyFailed = -3,
};

std::vector<std::pair<std::string, std::string>> GetTextureReplacement(std::string_view mipgenScript) noexcept;

int main(int argc, char* argv[])
{
	CmdParser parser;
	parser.AddString("material");
	parser.AddString("mipgen");
	parser.AddString("out");
	parser.AddString("log-dir");
	parser.AddString("log-file");
	parser.Parse(argc, argv);

	std::string_view logDir = parser.GetString("log-dir");
	std::string_view logFile = parser.GetString("log-file");
	Logger::SetLogsOuput(logDir.empty() ? Logger::GetDir() : logDir, logFile.empty() ? "log_MipGen.txt" : logFile);

	std::string_view materialFile = parser.GetString("material");
	if (materialFile.empty())
	{
		Logger::Error("No source material file specified!");
		return ResultCode::NoSourceFile;
	}
	std::string_view mipgen = parser.GetString("mipgen");
	std::string_view outFile = parser.GetString("out");

	if (mipgen.empty())
	{
		if (outFile.empty())
		{
			Logger::Error("No script with patching operation specified!");
			return ResultCode::NoMipgenScript;
		}
		Logger::InfoNoFile("No script with patching operation specified, performing simple copy.");
		Status error = {};
		std::filesystem::copy_file(materialFile, outFile, std::filesystem::copy_options::overwrite_existing, error);
		if (error)
		{
			ZE_CODE_ERROR(error, "Error copying material file!");
			return ResultCode::CopyFailed;
		}
		return ResultCode::Success;
	}
	if (outFile.empty())
		outFile = materialFile;
	
	std::vector<std::pair<std::string, std::string>> textureReplace = GetTextureReplacement(mipgen);
	return ResultCode::Success;
}

std::vector<std::pair<std::string, std::string>> GetTextureReplacement(std::string_view mipgenScript) noexcept
{
	std::vector<std::pair<std::string, std::string>> texturePairs;
	std::ifstream fin(mipgenScript.data());
	if (!fin.good())
		Logger::Error("Cannot open mipgen script \"" + std::string(mipgenScript) + "\"!");
	else
	{
		json::json jsonarray;
		fin >> jsonarray;
		if (jsonarray.is_array())
		{
			for (const auto& item : jsonarray)
			{
				if (item.contains("source") && item.contains("out"))
				{
					std::string source = jsonarray["source"].get<std::string>();
					std::string output = jsonarray["out"].get<std::string>();
					texturePairs.emplace_back(source, output);
				}
			}
		}
		else
		{
			if (jsonarray.contains("source") && jsonarray.contains("out"))
			{
				std::string source = jsonarray["source"].get<std::string>();
				std::string output = jsonarray["out"].get<std::string>();
				texturePairs.emplace_back(source, output);
			}
		}
	}
	return texturePairs;
}

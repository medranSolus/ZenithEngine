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
	NoWorkPerformed = -4,
	InvalidFileFormat = -5,
	ReadError = -6,
	WriteError = -7,
};

std::vector<std::pair<std::string, std::string>> GetTextureReplacement(std::string_view mipgenScript) noexcept;
ResultCode CopyMaterial(std::string_view materialFile, std::string_view outFile) noexcept;

int main(int argc, char* argv[])
{
	CmdParser parser;
	parser.AddOption("help-unique-texture-entries");
	parser.AddOption("unique-texture-entries");
	parser.AddString("material");
	parser.AddString("mipgen");
	parser.AddString("out");
	parser.AddString("log-dir");
	parser.AddString("log-file");
	if (parser.Parse(argc, argv))
		return ResultCode::Success;

	if (parser.GetOption("help-unique-texture-entries"))
	{
		Logger::InfoNoFile("When providing texture files to replace from the mipgen file, if each texture appears only once in the material, you cane use this flag to speed up matching of textures in the material file.");
		return ResultCode::Success;
	}

	std::string_view logDir = parser.GetString("log-dir");
	std::string_view logFile = parser.GetString("log-file");
	Logger::SetLogsOuput(logDir.empty() ? Logger::GetDir() : logDir, logFile.empty() ? "log_MatPatch.txt" : logFile);

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
		Logger::Warning("No script with patching operation specified, performing simple copy.");
		return CopyMaterial(materialFile, outFile);
	}
	if (outFile.empty())
		outFile = materialFile;
	
	std::vector<std::pair<std::string, std::string>> textureReplace = GetTextureReplacement(mipgen);
	if (textureReplace.empty())
	{
		Logger::Warning("No textures to replace in mipgen script, performing simple copy.");
		return CopyMaterial(materialFile, outFile);
	}

	std::filesystem::path path(materialFile);
	std::string ext = path.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](char c) { return static_cast<char>(std::tolower(c)); });

	std::ifstream fin(materialFile.data());
	if (!fin.good())
	{
		Logger::Error("Cannot open material file \"" + std::string(materialFile) + "\"!");
		return ResultCode::ReadError;
	}
	std::ofstream fout(outFile.data(), std::ios_base::out | std::ios_base::trunc);
	if (!fout.good())
	{
		Logger::Error("Cannot create output file \"" + std::string(outFile) + "\"!");
		return ResultCode::WriteError;
	}

	bool uniqueTex = parser.GetOption("unique-texture-entries");
	if (ext == ".mtl")
	{
		std::string line;
		bool first = true;
		while(std::getline(fin, line))
		{
			if (first)
				first = false;
			else
				fout << std::endl;
			for (auto it = textureReplace.begin(); it != textureReplace.end(); ++it)
			{
				const auto& source = it->first;
				U64 offset = line.find(source);
				if (offset != std::string::npos)
				{
					line.replace(offset, source.length(), it->second);
					if (uniqueTex)
						textureReplace.erase(it);
					break;
				}
			}
			fout << line;
		}
	}
	else
	{
		Logger::Error("Unsupported material file format \"" + std::string(materialFile) + "\"!");
		return ResultCode::InvalidFileFormat;
	}

	return ResultCode::Success;
}

ResultCode CopyMaterial(std::string_view materialFile, std::string_view outFile) noexcept
{
	Status error = {};
	std::filesystem::copy_file(materialFile, outFile, std::filesystem::copy_options::overwrite_existing, error);
	if (error)
	{
		ZE_CODE_ERROR(error, "Error copying material file!");
		return ResultCode::CopyFailed;
	}
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
					std::string source = item["source"].get<std::string>();
					std::string output = item["out"].get<std::string>();
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

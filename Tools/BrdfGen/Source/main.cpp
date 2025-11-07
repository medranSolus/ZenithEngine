#include "GFX/Surface.h"
#include "CmdParser.h"
#include "json.hpp"

namespace json = nlohmann;

enum ResultCode : int
{
	Success = 0,
	NoOutputFile = -1,
	CannotSaveFile = -2
};

ResultCode ProcessJsonCommand(const json::json& command) noexcept;
ResultCode RunJob(std::string_view output, U32 size, U32 samples, U32 cores, bool fp16) noexcept;

int main(int argc, char* argv[])
{
	ZE::CmdParser parser;
	parser.AddOption("fp16", 'f');
	parser.AddNumber("size", 1024, 's');
	parser.AddNumber("samples", 2048, 'n');
	parser.AddNumber("cores", 1, 'c');
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

	std::string_view output = parser.GetString("out");
	if (output.empty())
	{
		if (!json.empty())
			return ResultCode::Success;
		ZE::Logger::Error("No output file specified to generate LUT!");
		return ResultCode::NoOutputFile;
	}

	bool fp16 = parser.GetOption("fp16");
	U32 size = parser.GetNumber("size");
	U32 samples = parser.GetNumber("samples");
	U32 cores = parser.GetNumber("cores");

	return RunJob(output, size, samples, cores, fp16);
}

ResultCode ProcessJsonCommand(const json::json& command) noexcept
{
	std::string_view output = "";
	if (command.contains("out"))
		output = command["out"].get<std::string_view>();
	else
	{
		ZE::Logger::Error("JSON command missing required \"out\" parameter!");
		return ResultCode::NoOutputFile;
	}

	bool fp16 = false;
	if (command.contains("fp16"))
		fp16 = command["fp16"].get<bool>();
	U32 size = 1024;
	if (command.contains("size"))
		size = command["size"].get<U32>();
	U32 samples = 2048;
	if (command.contains("samples"))
		samples = command["samples"].get<U32>();
	U32 cores = 1;
	if (command.contains("cores"))
		cores = command["cores"].get<U32>();

	return RunJob(output, size, samples, cores, fp16);
}

ResultCode RunJob(std::string_view output, U32 size, U32 samples, U32 cores, bool fp16) noexcept
{
	ZE::Logger::InfoNoFile("Building BRDF LUT [" + std::to_string(size) + "x" + std::to_string(size) + "], "
		+ std::to_string(samples) + " samples, " + (fp16 ? "16 bit" : "32 bit") + ", output file: " + std::string(output));

	ZE::GFX::Surface lut(size, size, fp16 ? ZE::PixelFormat::R16G16_Float : ZE::PixelFormat::R32G32_Float);

	const float step = 1.0f / static_cast<float>(size);
	const U32 rowSize = lut.GetRowByteSize();
	U8* image = lut.GetBuffer();

	auto brdfGen = [step, rowSize, samples, fp16](U32 begin, U32 end, U32 size, U8* image)
		{
			for (U32 y = begin; y < end; ++y)
			{
				for (U32 x = 0; x < size; ++x)
				{
					const float NdotV = (static_cast<float>(x) + 0.5f) * step;
					const float roughness = (static_cast<float>(y) + 0.5f) * step;
					Float2 sample = ZE::Math::Light::IntegrateBRDF(NdotV, roughness, samples);

					if (fp16)
					{
						U32 packedValue = ZE::Math::FP16::EncodeFloat16(sample.x);
						packedValue |= static_cast<U32>(ZE::Math::FP16::EncodeFloat16(sample.y)) << 16;
						reinterpret_cast<U32*>(image)[x] = packedValue;
					}
					else
						reinterpret_cast<Float2*>(image)[x] = sample;
				}
				image += rowSize;
			}
		};

	if (cores > 1)
	{
		U32 jobRowCount = size / cores;
		--cores;
		std::vector<std::thread> workers;
		for (U8 i = 0; i < cores; ++i)
		{
			U32 jobOffset = i * jobRowCount;
			workers.emplace_back(brdfGen, jobOffset, jobOffset + jobRowCount, size, image);
			image += jobRowCount * rowSize;
		}
		brdfGen(jobRowCount * cores, size, size, image);
		for (auto& worker : workers)
			worker.join();
	}
	else
		brdfGen(0, size, size, image);

	if (!lut.Save(output))
	{
		ZE::Logger::Error("Cannot save BRDF LUT to file \"" + std::string(output) + "\"!");
		return ResultCode::CannotSaveFile;
	}
	return ResultCode::Success;
}
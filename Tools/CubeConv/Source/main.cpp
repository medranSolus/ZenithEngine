#include "GFX/Surface.h"
#include "CmdParser.h"
#include "json.hpp"

namespace json = nlohmann;
using namespace ZE;

enum ResultCode : int
{
	Success = 0,
	NoSourceFile = -1,
	NoOutputFile = -2,
	CannotLoadFile = -3,
	CannotSaveFile = -4,
	NotCubemap = -5,
};

ResultCode ProcessJsonCommand(const json::json& command) noexcept;
ResultCode RunJob(const std::vector<std::string_view>& source, std::string_view output, float sampleDelta, U32 convolutionSize, U32 cores, bool convFp16) noexcept;

int main(int argc, char* argv[])
{
	CmdParser parser;
	parser.AddOption("fp16");
	parser.AddOption("sample-delta-help");
	parser.AddNumber("sample-delta", 40, 'd');
	parser.AddNumber("cores", 1, 'c');
	parser.AddNumber("resize", 0, 'r');
	parser.AddString("source", "", 's');
	parser.AddString("source-px");
	parser.AddString("source-nx");
	parser.AddString("source-py");
	parser.AddString("source-ny");
	parser.AddString("source-pz");
	parser.AddString("source-nz");
	parser.AddString("out", "", 'o');
	parser.AddString("json", "", 'j');
	parser.Parse(argc, argv);

	if (parser.GetOption("sample-delta-help"))
	{
		Logger::InfoNoFile("Cubemap convolution sample delta parameter help:");
		Logger::InfoNoFile("  The sample delta controls the quality and speed of the cubemap convolution.");
		Logger::InfoNoFile("  It should be interpreted as 1/delta when used as step in sampling hemisphere around cubemap direction.");
		Logger::InfoNoFile("  The smaller inverse of the delta is, the better quality results can be achieved at the cost of execution speed.");
		Logger::InfoNoFile("  Default is 40 that corresponds to 0.025 sampling delta.");
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

	std::string_view output = parser.GetString("out");
	if (output.empty())
	{
		if (!json.empty())
			return ResultCode::Success;
		Logger::Error("No output file specified for cubemap convolution!");
		return ResultCode::NoOutputFile;
	}

	std::vector<std::string_view> sourceArray;
	std::string_view source = parser.GetString("source");
	if (source.empty())
	{
		bool hasAllFaces = true, anyFace = false;
		sourceArray.reserve(6);
		anyFace |= !sourceArray.emplace_back(parser.GetString("source-px")).empty();
		anyFace |= !sourceArray.emplace_back(parser.GetString("source-nx")).empty();
		anyFace |= !sourceArray.emplace_back(parser.GetString("source-py")).empty();
		anyFace |= !sourceArray.emplace_back(parser.GetString("source-ny")).empty();
		anyFace |= !sourceArray.emplace_back(parser.GetString("source-pz")).empty();
		anyFace |= !sourceArray.emplace_back(parser.GetString("source-nz")).empty();

		for (const auto& face : sourceArray)
		{
			if (face.empty())
			{
				hasAllFaces = false;
				break;
			}
		}

		if (!hasAllFaces)
		{
			if (anyFace)
				Logger::Error("Not all faces specified for cubemap to convolute!");
			else if (!json.empty())
				return ResultCode::Success;
			else
				Logger::Error("No source cubemap specified to convolute!");
			return ResultCode::NoSourceFile;
		}
	}
	else
		sourceArray.emplace_back(source);

	bool fp16 = parser.GetOption("fp16");
	U32 deltaParam = parser.GetNumber("sample-delta");
	float sampleDelta = deltaParam == 0 ? 0.025f : 1.0f / static_cast<float>(deltaParam);
	U32 cores = parser.GetNumber("cores");
	U32 resize = parser.GetNumber("resize");

	return RunJob(sourceArray, output, sampleDelta, resize, cores, fp16);
}

ResultCode ProcessJsonCommand(const json::json& command) noexcept
{
	std::string_view output = "";
	if (command.contains("out"))
		output = command["out"].get<std::string_view>();
	else
	{
		Logger::Error("JSON command missing required \"out\" parameter!");
		return ResultCode::NoOutputFile;
	}

	std::vector<std::string_view> sourceArray;
	if (command.contains("source"))
		sourceArray.emplace_back(command["source"].get<std::string_view>());
	else
	{
		bool hasAllFaces = true;
		hasAllFaces &= command.contains("source-px");
		hasAllFaces &= command.contains("source-nx");
		hasAllFaces &= command.contains("source-py");
		hasAllFaces &= command.contains("source-ny");
		hasAllFaces &= command.contains("source-pz");
		hasAllFaces &= command.contains("source-nz");

		if (hasAllFaces)
		{
			sourceArray.reserve(6);
			sourceArray.emplace_back(command["source-px"].get<std::string_view>());
			sourceArray.emplace_back(command["source-nx"].get<std::string_view>());
			sourceArray.emplace_back(command["source-py"].get<std::string_view>());
			sourceArray.emplace_back(command["source-ny"].get<std::string_view>());
			sourceArray.emplace_back(command["source-pz"].get<std::string_view>());
			sourceArray.emplace_back(command["source-nz"].get<std::string_view>());
		}
		else
		{
			bool anyFace = false;
			anyFace |= command.contains("source-px");
			anyFace |= command.contains("source-nx");
			anyFace |= command.contains("source-py");
			anyFace |= command.contains("source-ny");
			anyFace |= command.contains("source-pz");
			anyFace |= command.contains("source-nz");
			if (anyFace)
				Logger::Error("Not all faces specified for cubemap to convolute in JSON command!");
			else
				Logger::Error("JSON command missing required \"source\" parameter!");
			return ResultCode::NoSourceFile;
		}
	}

	bool fp16 = false;
	if (command.contains("fp16"))
		fp16 = command["fp16"].get<bool>();
	float sampleDelta = 0.025f;
	if (command.contains("sample-delta"))
		sampleDelta = command["sample-delta"].get<float>();
	U32 cores = 1;
	if (command.contains("cores"))
		cores = command["cores"].get<U32>();
	U32 resize = 0;
	if (command.contains("resize"))
		resize = command["resize"].get<U32>();

	return RunJob(sourceArray, output, sampleDelta, resize, cores, fp16);
}

ResultCode RunJob(const std::vector<std::string_view>& source, std::string_view output, float sampleDelta, U32 convolutionSize, U32 cores, bool convFp16) noexcept
{
	ZE_ASSERT(source.size(), "No source files specified for cubemap convolution!");
	Logger::InfoNoFile("Convoluting cubemap \"" + std::string(source.front()) + "\", output file: " + std::string(output));

	bool multipleSources = source.size() == 6;
	std::vector<GFX::Surface> cubemap;
	for (const auto& face : source)
	{
		if (!cubemap.emplace_back().Load(face))
		{
			Logger::Error("Cannot load file \"" + std::string(face) + "\"!");
			return ResultCode::CannotLoadFile;
		}
	}

	if ((source.size() != 1 && !multipleSources) || (source.size() == 1 && cubemap.front().GetArraySize() != 6))
	{
		if (multipleSources)
			Logger::Error("Source files do not form a cubemap!");
		else
			Logger::Error("Source file \"" + std::string(source.front()) + "\" is not a cubemap!");
		return ResultCode::NotCubemap;
	}

	const U32 cubemapSize = cubemap.front().GetWidth();
	if (multipleSources)
	{
		if (cubemap.front().GetHeight() != cubemapSize)
		{
			Logger::Error("Cubemap face \"" + std::string(source.front()) + "\" is not a square!");
			return ResultCode::NotCubemap;
		}

		const PixelFormat format = cubemap.front().GetFormat();
		if (format != PixelFormat::R16G16B16A16_Float && format != PixelFormat::R32G32B32A32_Float && format != PixelFormat::R32G32B32_Float)
		{
			Logger::Error("Cubemap face \"" + std::string(source.front()) + "\" is not in a supported HDR format (R16G16B16A16_Float or R32G32B32A32_Float or R32G32B32_Float)!");
			return ResultCode::NotCubemap;
		}

		for (U32 i = 1; i < 6; ++i)
		{
			if (cubemap.at(i).GetWidth() != cubemapSize || cubemap.at(i).GetHeight() != cubemapSize)
			{
				Logger::Error("Cubemap face \"" + std::string(source.at(i)) + "\" has different dimmensions than first face!");
				return ResultCode::NotCubemap;
			}
			if (format != cubemap.at(i).GetFormat())
			{
				Logger::Error("Cubemap face \"" + std::string(source.at(i)) + "\" has different pixel format than first face!");
				return ResultCode::NotCubemap;
			}
		}
	}
	else if (cubemap.front().GetWidth() != cubemap.front().GetHeight())
	{
		Logger::Error("Cubemap file \"" + std::string(source.front()) + "\" faces are not square!");
		return ResultCode::NotCubemap;
	}
	else
	{
		const PixelFormat format = cubemap.front().GetFormat();
		if (format != PixelFormat::R16G16B16A16_Float && format != PixelFormat::R32G32B32A32_Float && format != PixelFormat::R32G32B32_Float)
		{
			Logger::Error("Cubemap file \"" + std::string(source.front()) + "\" is not in a supported HDR format (R16G16B16A16_Float or R32G32B32A32_Float or R32G32B32_Float)!");
			return ResultCode::NotCubemap;
		}
	}

	if (convolutionSize != 0)
	{
		if (convolutionSize > cubemapSize)
		{
			Logger::Warning("Requested cubemap resize is larger than source cubemap size! Using source size.");
			convolutionSize = cubemapSize;
		}
		else
			Logger::InfoNoFile(" - Resizing cubemap to " + std::to_string(convolutionSize) + "x" + std::to_string(convolutionSize));
	}
	else
		convolutionSize = cubemapSize;

	const bool cubemapFp16 = Utils::GetChannelSize(cubemap.front().GetFormat()) == 2;
	GFX::Surface convolution(convolutionSize, convolutionSize, 1, 1, 6, convFp16 || cubemapFp16 ? PixelFormat::R16G16B16A16_Float : PixelFormat::R32G32B32_Float, false);

	std::vector<U8*> faces;
	faces.reserve(6);
	if (multipleSources)
	{
		for (auto& face : cubemap)
			faces.emplace_back(face.GetBuffer());
	}
	else
	{
		for (U16 i = 0; i < 6; ++i)
			faces.emplace_back(cubemap.front().GetImage(i, 0, 0));
	}

	const U64 sliceSize = convolution.GetSliceByteSize();
	const U32 rowSize = convolution.GetRowByteSize();
	const U8 pixelSize = convolution.GetPixelSize();
	const U32 cubemapRowSize = cubemap.front().GetRowByteSize();
	const U32 cubemapPixelSize = cubemap.front().GetPixelSize();
	const Vector up = Math::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	auto convolute = [&](U8* image, U16 begin, U16 end, U32 beginRow, U32 endRow)
		{
			for (U16 a = begin; a < end; ++a)
			{
				const Math::CubemapFaceTraversalDesc& faceDesc = Math::CUBEMAP_FACES_INFO.at(a);
				const Vector faceStart = Math::XMLoadFloat3(&faceDesc.StartPos);
				const Vector xDir = Math::XMLoadFloat3(&faceDesc.DirX);
				const Vector yDir = Math::XMLoadFloat3(&faceDesc.DirY);

				for (U32 y = a == begin ? beginRow : 0, rows = a == end - 1 ? endRow : convolutionSize; y < rows; ++y)
				{
					const Vector yScale = Math::XMVectorReplicate((static_cast<float>(y) + 0.5f) / static_cast<float>(convolutionSize));
					const Vector rowPos = Math::XMVectorMultiplyAdd(yDir, yScale, faceStart);

					for (U32 x = 0; x < convolutionSize; ++x)
					{
						const Vector xScale = Math::XMVectorReplicate((static_cast<float>(x) + 0.5f) / static_cast<float>(convolutionSize));
						const Vector hemisphereNormal = Math::XMVector3Normalize(Math::XMVectorMultiplyAdd(xDir, xScale, rowPos));
						const Vector tan = Math::XMVector3Normalize(Math::XMVector3Cross(up, hemisphereNormal));
						const Vector bitan = Math::XMVector3Normalize(Math::XMVector3Cross(hemisphereNormal, tan));

						U64 samples = 0;
						Vector irradiance = { 0.0f, 0.0f, 0.0f, 0.0f };

						for (float phi = 0.0f; phi < Math::PI2; phi += sampleDelta)
						{
							const float phiSin = std::sinf(phi);
							const float phiCos = std::cosf(phi);
							for (float theta = sampleDelta; theta < static_cast<float>(M_PI_2); theta += sampleDelta)
							{
								// Spherical to cartesian in tangent space
								const float thetaSin = std::sinf(theta);
								const float thetaCos = std::cosf(theta);
								if (thetaSin == 0.0f || thetaCos == 0.0f)
									continue;

								// Tangent space to world
								Vector sampleDir = Math::XMVector3Normalize(Math::XMVectorMultiplyAdd(tan, Math::XMVectorReplicate(thetaSin * phiCos),
									Math::XMVectorMultiplyAdd(bitan, Math::XMVectorReplicate(thetaSin * phiSin),
										Math::XMVectorMultiply(hemisphereNormal, Math::XMVectorReplicate(thetaCos)))));

								const float sampleX = Math::XMVectorGetX(sampleDir);
								const float sampleY = Math::XMVectorGetY(sampleDir);
								const float sampleZ = Math::XMVectorGetZ(sampleDir);

								// Find which face to sample from
								Vector sampleAbs = Math::XMVectorAbs(sampleDir);

								U8* sampleFace = nullptr;
								Vector uvCoords = {};
								float uvFactor = 0.5f;
								if (Math::XMComparisonAllTrue(Math::XMVector3GreaterOrEqualR(Math::XMVectorSplatY(sampleAbs), sampleAbs)))
								{
									uvFactor /= Math::XMVectorGetY(sampleAbs);

									float coordY = 0.0f;
									if (sampleY >= 0.0f)
									{
										sampleFace = faces.at(2);
										coordY = sampleZ;
									}
									else
									{
										sampleFace = faces.at(3);
										coordY = -sampleZ;
									}
									uvCoords = Math::XMVectorSet(sampleX, coordY, 0.0f, 0.0f);
								}
								else
								{
									float coordX = 0.0f;
									if (Math::XMComparisonAllTrue(Math::XMVector3GreaterOrEqualR(Math::XMVectorSplatX(sampleAbs), sampleAbs)))
									{
										uvFactor /= Math::XMVectorGetX(sampleAbs);

										if (sampleX >= 0.0f)
										{
											sampleFace = faces.at(0);
											coordX = -sampleZ;
										}
										else
										{
											sampleFace = faces.at(1);
											coordX = sampleZ;
										}
									}
									else
									{
										uvFactor /= Math::XMVectorGetZ(sampleAbs);

										if (sampleZ >= 0.0f)
										{
											sampleFace = faces.at(4);
											coordX = sampleX;
										}
										else
										{
											sampleFace = faces.at(5);
											coordX = -sampleX;
										}
									}
									uvCoords = Math::XMVectorSet(coordX, -sampleY, 0.0f, 0.0f);
								}
								uvCoords = Math::XMVectorMultiplyAdd(uvCoords, Math::XMVectorReplicate(uvFactor), Math::XMVectorReplicate(0.5f));
								uvCoords = Math::XMVectorMultiply(Math::XMVectorSetX(uvCoords, 1.0f - Math::XMVectorGetX(uvCoords)), Math::XMVectorReplicate(static_cast<float>(cubemapSize - 1)));

								const U64 sampleFaceX = Utils::SafeCast<U64>(Math::XMVectorGetX(uvCoords));
								const U64 sampleFaceY = Utils::SafeCast<U64>(Math::XMVectorGetY(uvCoords));

								Vector sample = {};
								const U64 sampleOffset = sampleFaceY * cubemapRowSize + sampleFaceX * cubemapPixelSize;
								if (cubemapFp16)
								{
									sample = Math::XMVectorSet(
										Math::FP16::DecodeFloat16(*reinterpret_cast<U16*>(sampleFace + sampleOffset)),
										Math::FP16::DecodeFloat16(*reinterpret_cast<U16*>(sampleFace + sampleOffset + 2)),
										Math::FP16::DecodeFloat16(*reinterpret_cast<U16*>(sampleFace + sampleOffset + 4)),
										0.0f);
								}
								else
									sample = Math::XMLoadFloat3(reinterpret_cast<Float3*>(sampleFace + sampleOffset)); // Alpha can be ignored since only RGB is important

								irradiance = Math::XMVectorMultiplyAdd(sample, Math::XMVectorReplicate(thetaSin * thetaCos), irradiance);
								++samples;
							}
						}

						irradiance = Math::XMVectorMultiply(irradiance, Math::XMVectorReplicate(Math::PI / static_cast<float>(samples)));

						const U64 convolutionOffset = Utils::SafeCast<U64>(y) * rowSize + Utils::SafeCast<U64>(x) * pixelSize;
						if (convFp16)
						{
							*reinterpret_cast<U16*>(image + convolutionOffset) = Math::FP16::EncodeFloat16(Math::XMVectorGetX(irradiance));
							*reinterpret_cast<U16*>(image + convolutionOffset + 2) = Math::FP16::EncodeFloat16(Math::XMVectorGetY(irradiance));
							*reinterpret_cast<U16*>(image + convolutionOffset + 4) = Math::FP16::EncodeFloat16(Math::XMVectorGetZ(irradiance));
							*reinterpret_cast<U16*>(image + convolutionOffset + 6) = Math::FP16::EncodeFloat16(0.0f);
						}
						else
							Math::XMStoreFloat3(reinterpret_cast<Float3*>(image + convolutionOffset), irradiance);
					}
				}
				image += sliceSize;
			}
		};

	U8* convolutionBuffer = convolution.GetBuffer();
	if (cores > 1)
	{
		const U32 jobsCount = convolutionSize * 6;
		cores = std::clamp(cores, 2U, jobsCount);
		const U32 batchSize = jobsCount / cores;
		--cores;

		U16 arrayOffset = 0;
		U32 rowOffset = 0;
		std::vector<std::thread> workers;
		for (U32 i = 0; i < cores; ++i)
		{
			U16 arrayCount = 1;
			U32 rowCount = batchSize;
			while (rowCount > convolutionSize)
			{
				++arrayCount;
				rowCount -= convolutionSize;
			}
			rowCount += rowOffset;
			if (rowCount > convolutionSize)
			{
				++arrayCount;
				rowCount -= convolutionSize;
			}

			workers.emplace_back(convolute, convolutionBuffer, arrayOffset, static_cast<U16>(arrayOffset + arrayCount), rowOffset, rowCount);

			if (rowCount == convolutionSize)
				rowCount = 0;
			else
				--arrayCount;
			convolutionBuffer += arrayCount * sliceSize;
			arrayOffset += arrayCount;
			rowOffset = rowCount;
		}
		convolute(convolutionBuffer, arrayOffset, 6, rowOffset, convolutionSize);
		for (auto& worker : workers)
			worker.join();
	}
	else
		convolute(convolutionBuffer, 0, 6, 0, convolutionSize);

	if (!convolution.Save(output))
	{
		Logger::Error("Cannot save convoluted cubemap to file \"" + std::string(output) + "\"!");
		return ResultCode::CannotSaveFile;
	}
	return ResultCode::Success;
}
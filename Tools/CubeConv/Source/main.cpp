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

struct ConvolutionParams
{
	std::string_view OutputFile = "";
	std::vector<std::string_view> SourceFiles;
	bool Fp16 = false;
	bool Specular = false;
	bool SampleMipAdjst = false;
	float SampleDelta = 0.025f;
	U32 SamplesCount = 1024;
	U32 Cores = 1;
	U32 ConvolutionSize = 0;
};

ResultCode ProcessJsonCommand(const json::json& command) noexcept;
ResultCode RunJob(ConvolutionParams& params) noexcept;
void ConvoluteIrradiance(GFX::Surface& convolution, const std::vector<U8*>& faces, const std::vector<GFX::Surface>& cubemap, ConvolutionParams& params) noexcept;
void ConvolutePrefiltered(GFX::Surface& convolution, const std::vector<U8*>& faces, const std::vector<GFX::Surface>& cubemap, ConvolutionParams& params) noexcept;

int main(int argc, char* argv[])
{
	CmdParser parser;
	parser.AddOption("fp16");
	parser.AddOption("specular");
	parser.AddOption("sample-mip-adjst");
	parser.AddOption("help-sample-delta");
	parser.AddOption("help-samples-count");
	parser.AddOption("help-sample-mip-adjst");
	parser.AddFloat("sample-delta", 0.025f, 'd');
	parser.AddNumber("samples-count", 1024);
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

	if (parser.GetOption("help-sample-delta"))
	{
		Logger::InfoNoFile("Cubemap irradiance convolution sample delta parameter help:");
		Logger::InfoNoFile("  The sample delta controls the quality and speed of the cubemap convolution.");
		Logger::InfoNoFile("  The smaller inverse of the delta is, the better quality results can be achieved at the cost of execution speed.");
		Logger::InfoNoFile("  Not used when <specular> option is specified.");
		return ResultCode::Success;
	}

	if (parser.GetOption("help-samples-count"))
	{
		Logger::InfoNoFile("Cubemap prefiltering convolution samples count parameter help:");
		Logger::InfoNoFile("  Number of samples to be performed per texel of the cubemap during prefiltering for specular response.");
		Logger::InfoNoFile("  Only used when <specular> option is specified.");
		return ResultCode::Success;
	}

	if (parser.GetOption("help-sample-mip-adjst"))
	{
		Logger::InfoNoFile("Cubemap prefiltering convolution sample mip level adjustment help:");
		Logger::InfoNoFile("  Use this method of sampling original environment map when when light levels are varying highly across the map to reduce the artifacts in convolution.");
		Logger::InfoNoFile("  Please note that this requires full mip chain to be generated for given environment map.");
		Logger::InfoNoFile("  Only used when <specular> option is specified.");
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

	ConvolutionParams params = {};
	params.OutputFile = parser.GetString("out");
	if (params.OutputFile.empty())
	{
		if (!json.empty())
			return ResultCode::Success;
		Logger::Error("No output file specified for cubemap convolution!");
		return ResultCode::NoOutputFile;
	}

	std::string_view source = parser.GetString("source");
	if (source.empty())
	{
		bool hasAllFaces = true, anyFace = false;
		params.SourceFiles.reserve(6);
		anyFace |= !params.SourceFiles.emplace_back(parser.GetString("source-px")).empty();
		anyFace |= !params.SourceFiles.emplace_back(parser.GetString("source-nx")).empty();
		anyFace |= !params.SourceFiles.emplace_back(parser.GetString("source-py")).empty();
		anyFace |= !params.SourceFiles.emplace_back(parser.GetString("source-ny")).empty();
		anyFace |= !params.SourceFiles.emplace_back(parser.GetString("source-pz")).empty();
		anyFace |= !params.SourceFiles.emplace_back(parser.GetString("source-nz")).empty();

		for (const auto& face : params.SourceFiles)
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
		params.SourceFiles.emplace_back(source);

	params.Fp16 = parser.GetOption("fp16");
	params.Specular = parser.GetOption("specular");
	params.SampleMipAdjst = parser.GetOption("sample-mip-adjst");
	params.SampleDelta = parser.GetFloat("sample-delta");
	params.SamplesCount = parser.GetNumber("samples-count");
	params.Cores = parser.GetNumber("cores");
	params.ConvolutionSize = parser.GetNumber("resize");

	return RunJob(params);
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

	ConvolutionParams params = {};
	if (command.contains("fp16"))
		params.Fp16 = command["fp16"].get<bool>();
	if (command.contains("specular"))
		params.Specular = command["specular"].get<bool>();
	if (command.contains("sample-mip-adjst"))
		params.SampleMipAdjst = command["sample-mip-adjst"].get<bool>();
	if (command.contains("sample-delta"))
		params.SampleDelta = command["sample-delta"].get<float>();
	if (command.contains("samples-count"))
		params.SamplesCount = command["samples-count"].get<U32>();
	if (command.contains("cores"))
		params.Cores = command["cores"].get<U32>();
	if (command.contains("resize"))
		params.ConvolutionSize = command["resize"].get<U32>();

	return RunJob(params);
}

ResultCode RunJob(ConvolutionParams& params) noexcept
{
	if (params.SampleDelta <= FLT_EPSILON)
	{
		Logger::Warning("Sample delta must be greater than 0! Using default value 0.025.");
		params.SampleDelta = 0.025f;
	}
	if (params.SamplesCount == 0)
	{
		Logger::Warning("Samples count cannot be 0! Using default value 1024.");
		params.SamplesCount = 1024;
	}

	ZE_ASSERT(params.SourceFiles.size(), "No source files specified for cubemap convolution!");
	Logger::InfoNoFile("Convoluting cubemap \"" + std::string(params.SourceFiles.front()) + "\", output file: " + std::string(params.OutputFile));

	bool multipleSources = params.SourceFiles.size() == 6;
	std::vector<GFX::Surface> cubemap;
	for (const auto& face : params.SourceFiles)
	{
		if (!cubemap.emplace_back().Load(face))
		{
			Logger::Error("Cannot load file \"" + std::string(face) + "\"!");
			return ResultCode::CannotLoadFile;
		}
	}

	if ((params.SourceFiles.size() != 1 && !multipleSources) || (params.SourceFiles.size() == 1 && cubemap.front().GetArraySize() != 6))
	{
		if (multipleSources)
			Logger::Error("Source files do not form a cubemap!");
		else
			Logger::Error("Source file \"" + std::string(params.SourceFiles.front()) + "\" is not a cubemap!");
		return ResultCode::NotCubemap;
	}

	const U32 cubemapSize = cubemap.front().GetWidth();
	if (multipleSources)
	{
		if (cubemap.front().GetHeight() != cubemapSize)
		{
			Logger::Error("Cubemap face \"" + std::string(params.SourceFiles.front()) + "\" is not a square!");
			return ResultCode::NotCubemap;
		}

		const PixelFormat format = cubemap.front().GetFormat();
		if (format != PixelFormat::R16G16B16A16_Float && format != PixelFormat::R32G32B32A32_Float && format != PixelFormat::R32G32B32_Float)
		{
			Logger::Error("Cubemap face \"" + std::string(params.SourceFiles.front()) + "\" is not in a supported HDR format (R16G16B16A16_Float or R32G32B32A32_Float or R32G32B32_Float)!");
			return ResultCode::NotCubemap;
		}

		for (U32 i = 1; i < 6; ++i)
		{
			if (cubemap.at(i).GetWidth() != cubemapSize || cubemap.at(i).GetHeight() != cubemapSize)
			{
				Logger::Error("Cubemap face \"" + std::string(params.SourceFiles.at(i)) + "\" has different dimmensions than first face!");
				return ResultCode::NotCubemap;
			}
			if (format != cubemap.at(i).GetFormat())
			{
				Logger::Error("Cubemap face \"" + std::string(params.SourceFiles.at(i)) + "\" has different pixel format than first face!");
				return ResultCode::NotCubemap;
			}
		}
	}
	else if (cubemap.front().GetWidth() != cubemap.front().GetHeight())
	{
		Logger::Error("Cubemap file \"" + std::string(params.SourceFiles.front()) + "\" faces are not square!");
		return ResultCode::NotCubemap;
	}
	else
	{
		const PixelFormat format = cubemap.front().GetFormat();
		if (format != PixelFormat::R16G16B16A16_Float && format != PixelFormat::R32G32B32A32_Float && format != PixelFormat::R32G32B32_Float)
		{
			Logger::Error("Cubemap file \"" + std::string(params.SourceFiles.front()) + "\" is not in a supported HDR format (R16G16B16A16_Float or R32G32B32A32_Float or R32G32B32_Float)!");
			return ResultCode::NotCubemap;
		}
	}

	if (params.ConvolutionSize != 0)
	{
		if (params.ConvolutionSize > cubemapSize)
		{
			Logger::Warning("Requested cubemap resize is larger than source cubemap size! Using source size.");
			params.ConvolutionSize = cubemapSize;
		}
		else
			Logger::InfoNoFile(" - Resizing cubemap to " + std::to_string(params.ConvolutionSize) + "x" + std::to_string(params.ConvolutionSize));
	}
	else
		params.ConvolutionSize = cubemapSize;

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

	PixelFormat format = PixelFormat::R32G32B32_Float;
	if (params.Fp16 || Utils::GetChannelSize(cubemap.front().GetFormat()) == 2)
		format = PixelFormat::R16G16B16A16_Float;
	U16 mipLevels = 1;
	if (params.Specular)
		mipLevels = Math::GetMipLevels(params.ConvolutionSize, params.ConvolutionSize);

	GFX::Surface convolution(params.ConvolutionSize, params.ConvolutionSize, 1, mipLevels, 6, format, false);

	if (params.Specular)
		ConvolutePrefiltered(convolution, faces, cubemap, params);
	else
		ConvoluteIrradiance(convolution, faces, cubemap, params);

	if (!convolution.Save(params.OutputFile))
	{
		Logger::Error("Cannot save convoluted cubemap to file \"" + std::string(params.OutputFile) + "\"!");
		return ResultCode::CannotSaveFile;
	}
	return ResultCode::Success;
}

void ConvoluteIrradiance(GFX::Surface& convolution, const std::vector<U8*>& faces, const std::vector<GFX::Surface>& cubemap, ConvolutionParams& params) noexcept
{
	const bool convFp16 = Utils::GetChannelSize(convolution.GetFormat()) == 2;
	const U64 sliceSize = convolution.GetSliceByteSize();
	const U32 rowSize = convolution.GetRowByteSize();
	const U8 pixelSize = convolution.GetPixelSize();

	const bool cubemapFp16 = Utils::GetChannelSize(cubemap.front().GetFormat()) == 2;
	const U32 cubemapSize = cubemap.front().GetWidth();
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

				for (U32 y = a == begin ? beginRow : 0, rows = a == end - 1 ? endRow : params.ConvolutionSize; y < rows; ++y)
				{
					const Vector yScale = Math::XMVectorReplicate((static_cast<float>(y) + 0.5f) / static_cast<float>(params.ConvolutionSize));
					const Vector rowPos = Math::XMVectorMultiplyAdd(yDir, yScale, faceStart);

					for (U32 x = 0; x < params.ConvolutionSize; ++x)
					{
						const Vector xScale = Math::XMVectorReplicate((static_cast<float>(x) + 0.5f) / static_cast<float>(params.ConvolutionSize));
						const Vector hemisphereNormal = Math::XMVector3Normalize(Math::XMVectorMultiplyAdd(xDir, xScale, rowPos));
						Vector tan = Math::XMVector3Normalize(Math::XMVector3Cross(up, hemisphereNormal));
						const Vector bitan = Math::XMVector3Normalize(Math::XMVector3Cross(hemisphereNormal, tan));
						tan = Math::XMVector3Normalize(Math::XMVector3Cross(bitan, hemisphereNormal));

						U64 samples = 0;
						Vector irradiance = { 0.0f, 0.0f, 0.0f, 0.0f };

						for (float phi = 0.0f; phi < Math::PI2; phi += params.SampleDelta)
						{
							const float phiSin = std::sinf(phi);
							const float phiCos = std::cosf(phi);
							for (float theta = params.SampleDelta; theta < static_cast<float>(M_PI_2); theta += params.SampleDelta)
							{
								// Spherical to cartesian in tangent space
								const float thetaSin = std::sinf(theta);
								const float thetaCos = std::cosf(theta);
								if (thetaSin == 0.0f || thetaCos == 0.0f)
									continue;

								// Tangent space to world
								const Vector sampleDir = Math::XMVector3Normalize(Math::XMVectorMultiplyAdd(tan, Math::XMVectorReplicate(thetaSin * phiCos),
									Math::XMVectorMultiplyAdd(bitan, Math::XMVectorReplicate(thetaSin * phiSin),
										Math::XMVectorMultiply(hemisphereNormal, Math::XMVectorReplicate(thetaCos)))));

								UInt3 sampledFace = Math::SampleCubemap(sampleDir, cubemapSize);
								U8* sampleFace = faces.at(sampledFace.Z);

								Vector sample = {};
								const U64 sampleOffset = static_cast<U64>(sampledFace.Y) * cubemapRowSize + static_cast<U64>(sampledFace.X) * cubemapPixelSize;
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
	if (params.Cores > 1)
	{
		const U32 jobsCount = params.ConvolutionSize * 6;
		params.Cores = std::clamp(params.Cores, 2U, jobsCount);
		const U32 batchSize = jobsCount / params.Cores;
		--params.Cores;

		U16 arrayOffset = 0;
		U32 rowOffset = 0;
		std::vector<std::thread> workers;
		workers.reserve(params.Cores);
		for (U32 i = 0; i < params.Cores; ++i)
		{
			U16 arrayCount = 1;
			U32 rowCount = batchSize;
			while (rowCount > params.ConvolutionSize)
			{
				++arrayCount;
				rowCount -= params.ConvolutionSize;
			}
			rowCount += rowOffset;
			if (rowCount > params.ConvolutionSize)
			{
				++arrayCount;
				rowCount -= params.ConvolutionSize;
			}

			workers.emplace_back(convolute, convolutionBuffer, arrayOffset, static_cast<U16>(arrayOffset + arrayCount), rowOffset, rowCount);

			if (rowCount == params.ConvolutionSize)
				rowCount = 0;
			else
				--arrayCount;
			convolutionBuffer += arrayCount * sliceSize;
			arrayOffset += arrayCount;
			rowOffset = rowCount;
		}
		convolute(convolutionBuffer, arrayOffset, 6, rowOffset, params.ConvolutionSize);
		for (auto& worker : workers)
			worker.join();
	}
	else
		convolute(convolutionBuffer, 0, 6, 0, params.ConvolutionSize);
}

void ConvolutePrefiltered(GFX::Surface& convolution, const std::vector<U8*>& faces, const std::vector<GFX::Surface>& cubemap, ConvolutionParams& params) noexcept
{
	const bool convFp16 = Utils::GetChannelSize(convolution.GetFormat()) == 2;
	const U8 pixelSize = convolution.GetPixelSize();
	const U16 mipLevels = convolution.GetMipCount();

	const PixelFormat cubemapFormat = cubemap.front().GetFormat();
	const bool cubemapFp16 = Utils::GetChannelSize(cubemapFormat) == 2;
	const U32 cubemapSize = cubemap.front().GetWidth();
	const U32 cubemapRowSize = cubemap.front().GetRowByteSize();
	const U32 cubemapPixelSize = cubemap.front().GetPixelSize();
	const U16 cubemapMipLevels = cubemap.front().GetMipCount();

	auto convolute = [&](U16 startMip, U16 mipCount, U32 startRow, U32 rowCount)
		{
			U8* convolutionBuffer = convolution.GetBuffer();
			for (U16 a = 0; a < 6; ++a)
			{
				const Math::CubemapFaceTraversalDesc& faceDesc = Math::CUBEMAP_FACES_INFO.at(a);
				const Vector faceStart = Math::XMLoadFloat3(&faceDesc.StartPos);
				const Vector xDir = Math::XMLoadFloat3(&faceDesc.DirX);
				const Vector yDir = Math::XMLoadFloat3(&faceDesc.DirY);

				for (U16 mip = startMip, endMip = startMip + mipCount; mip < endMip; ++mip)
				{
					if (mipCount == 1)
						convolutionBuffer = convolution.GetImage(a, mip, 0);

					const float roughness = static_cast<float>(mip) / static_cast<float>(std::max(mipLevels - 1U, 1U));
					const U64 sliceSize = convolution.GetSliceByteSize(mip);
					const U32 rowSize = convolution.GetRowByteSize(mip);

					const U32 mipWidth = std::max(params.ConvolutionSize >> mip, 1U);
					const U32 mipHeight = mipCount == 1 ? startRow + rowCount : mipWidth;

					for (U32 y = startRow; y < mipHeight; ++y)
					{
						const Vector yScale = Math::XMVectorReplicate((static_cast<float>(y) + 0.5f) / static_cast<float>(mipWidth));
						const Vector rowPos = Math::XMVectorMultiplyAdd(yDir, yScale, faceStart);

						for (U32 x = 0; x < mipWidth; ++x)
						{
							const Vector xScale = Math::XMVectorReplicate((static_cast<float>(x) + 0.5f) / static_cast<float>(mipWidth));
							const Vector direction = Math::XMVector3Normalize(Math::XMVectorMultiplyAdd(xDir, xScale, rowPos));

							Vector prefilteredColor = { 0.0f, 0.0f, 0.0f, 0.0f };
							float totalWeight = 0.0f;
							for (U32 i = 0; i < params.SamplesCount; ++i)
							{
								// L direction wrong in x
								const Float2 Xi = Math::Light::HammersleySequence(i, params.SamplesCount);
								const Vector H = Math::Light::ImportanceSampleGGX(Xi, roughness, direction);
								const Vector NdotH = Math::XMVectorMax(Math::XMVector3Dot(direction, H), Math::XMVectorZero());
								const Vector L = Math::XMVector3Normalize(Math::XMVectorMultiplyAdd(Math::XMVectorReplicate(2.0f),
									Math::XMVectorMultiply(NdotH, H), Math::XMVectorNegate(direction)));

								const Vector NdotL = Math::XMVectorMax(Math::XMVector3Dot(direction, L), Math::XMVectorZero());
								const float weight = Math::XMVectorGetX(NdotL);
								if (weight > 0.0f)
								{
									Vector sample = {};
									if (params.SampleMipAdjst && roughness != 0.0f)
									{
										U32 faceIndex = UINT32_MAX;
										const Float2 sampleUV = Math::SampleCubemapUV(L, faceIndex);

										const float pdf = (Math::Light::GeometrySchlickGGX(Math::XMVectorGetX(NdotH), roughness) * 0.25f) + 0.0001f;
										const float mipLevel = 0.5f * std::log2((3.0f * static_cast<float>(cubemapSize * cubemapSize)) / (Math::PI2 * (static_cast<float>(params.SamplesCount) * pdf + 0.0001f)));

										float mipIntegral = 0.0f;
										const float factor = std::modf(mipLevel, &mipIntegral);
										const U16 lowerMip = std::min(Utils::SafeCast<U16>(mipIntegral), cubemapMipLevels);
										const U16 higherMip = std::min<U16>(lowerMip + 1U, cubemapMipLevels);

										const U32 lowSize = std::max(cubemapSize >> lowerMip, 1U);
										const UInt2 lowSampleCoord = { Utils::SafeCast<U32>(sampleUV.x * static_cast<float>(lowSize)), Utils::SafeCast<U32>(sampleUV.y * static_cast<float>(lowSize)) };

										const U32 lowByteSize = cubemap.front().GetRowByteSize(lowerMip);
										const U64 lowOffset = static_cast<U64>(lowSampleCoord.Y) * lowByteSize + static_cast<U64>(lowSampleCoord.X) * cubemapPixelSize;
										U8* lowSampleFace = faces.at(faceIndex) + GFX::Surface::GetMipOffset(cubemapSize, cubemapSize, 1, cubemapFormat, lowerMip, 0);

										Vector lowSample = {};
										if (cubemapFp16)
										{
											lowSample = Math::XMVectorSet(
												Math::FP16::DecodeFloat16(*reinterpret_cast<U16*>(lowSampleFace + lowOffset)),
												Math::FP16::DecodeFloat16(*reinterpret_cast<U16*>(lowSampleFace + lowOffset + 2)),
												Math::FP16::DecodeFloat16(*reinterpret_cast<U16*>(lowSampleFace + lowOffset + 4)),
												0.0f);
										}
										else
											lowSample = Math::XMLoadFloat3(reinterpret_cast<Float3*>(lowSampleFace + lowOffset));

										if (lowerMip == higherMip)
											sample = lowSample;
										else
										{
											const U32 highSize = std::max(cubemapSize >> higherMip, 1U);
											const UInt2 highSampleCoord = { Utils::SafeCast<U32>(sampleUV.x * static_cast<float>(highSize)), Utils::SafeCast<U32>(sampleUV.y * static_cast<float>(highSize)) };

											const U32 highByteSize = cubemap.front().GetRowByteSize(higherMip);
											const U64 highOffset = static_cast<U64>(highSampleCoord.Y) * highByteSize + static_cast<U64>(highSampleCoord.X) * cubemapPixelSize;
											U8* highSampleFace = lowSampleFace + GFX::Surface::GetSliceByteSize(cubemapSize, cubemapSize, cubemapFormat, higherMip);

											Vector highSample = {};
											if (cubemapFp16)
											{
												highSample = Math::XMVectorSet(
													Math::FP16::DecodeFloat16(*reinterpret_cast<U16*>(highSampleFace + highOffset)),
													Math::FP16::DecodeFloat16(*reinterpret_cast<U16*>(highSampleFace + highOffset + 2)),
													Math::FP16::DecodeFloat16(*reinterpret_cast<U16*>(highSampleFace + highOffset + 4)),
													0.0f);
											}
											else
												highSample = Math::XMLoadFloat3(reinterpret_cast<Float3*>(highSampleFace + highOffset));

											sample = Math::XMVectorLerp(lowSample, highSample, factor);
										}
									}
									else
									{
										UInt3 sampledFace = Math::SampleCubemap(L, cubemapSize);
										U8* sampleFace = faces.at(sampledFace.Z);

										const U64 sampleOffset = static_cast<U64>(sampledFace.Y) * cubemapRowSize + static_cast<U64>(sampledFace.X) * cubemapPixelSize;
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
									}

									prefilteredColor = Math::XMVectorMultiplyAdd(sample, NdotL, prefilteredColor);
									totalWeight += weight;
								}
							}

							prefilteredColor = Math::XMVectorDivide(prefilteredColor, Math::XMVectorReplicate(totalWeight));

							const U64 convolutionOffset = Utils::SafeCast<U64>(y) * rowSize + Utils::SafeCast<U64>(x) * pixelSize;
							if (convFp16)
							{
								*reinterpret_cast<U16*>(convolutionBuffer + convolutionOffset) = Math::FP16::EncodeFloat16(Math::XMVectorGetX(prefilteredColor));
								*reinterpret_cast<U16*>(convolutionBuffer + convolutionOffset + 2) = Math::FP16::EncodeFloat16(Math::XMVectorGetY(prefilteredColor));
								*reinterpret_cast<U16*>(convolutionBuffer + convolutionOffset + 4) = Math::FP16::EncodeFloat16(Math::XMVectorGetZ(prefilteredColor));
								*reinterpret_cast<U16*>(convolutionBuffer + convolutionOffset + 6) = Math::FP16::EncodeFloat16(0.0f);
							}
							else
								Math::XMStoreFloat3(reinterpret_cast<Float3*>(convolutionBuffer + convolutionOffset), prefilteredColor);
						}
					}

					if (mipCount != 1)
						convolutionBuffer += sliceSize;
				}
			}
		};

	if (params.Cores > 1)
	{
		struct ThreadTask
		{
			U16 MipLevel;
			U32 RowStart;
			U32 RowCount;
		};

		std::vector<ThreadTask> tasks;
		for (U16 mip = 0; mip < mipLevels; ++mip)
		{
			U32 rowCount = std::max(params.ConvolutionSize >> mip, 1U);
			// Don't subdivide for small mips too much
			if (rowCount < 16)
				tasks.emplace_back(mip, 0, rowCount);
			else
			{
				U32 mipWorkers = std::clamp(params.Cores, 1U, rowCount);
				U32 rowsPerTask = rowCount / mipWorkers;
				ThreadTask task = { mip, 0, rowsPerTask };

				while (rowCount > rowsPerTask)
				{
					tasks.emplace_back(task);
					task.RowStart += rowsPerTask;
					rowCount -= rowsPerTask;
				}
				task.RowCount = rowCount;
				tasks.emplace_back(task);
			}
		}

		std::vector<std::thread> workers;
		workers.reserve(params.Cores - 1);
		auto workerFunc = [&](U32 id)
			{
				for (U32 taskIdx = id; taskIdx < tasks.size(); taskIdx += params.Cores)
				{
					const auto& task = tasks.at(taskIdx);
					convolute(task.MipLevel, 1, task.RowStart, task.RowCount);
				}
			};
		for (U32 i = 1; i < params.Cores; ++i)
			workers.emplace_back(workerFunc, i);
		workerFunc(0);

		for (auto& worker : workers)
			worker.join();
	}
	else
		convolute(0, mipLevels, 0, 0);
}
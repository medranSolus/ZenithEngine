#include "TexOps.h"

namespace TexOps
{
	void SimpleProcess(GFX::Surface& surface, U32 cores, bool noAlpha, bool flipY) noexcept
	{
		// TODO: Multithreaded version
		const U8 channelSize = Utils::GetChannelSize(surface.GetFormat());
		const U8 pixelSize = surface.GetPixelSize();
		U8* buffer = surface.GetBuffer();
		for (U16 a = 0; a < surface.GetArraySize(); ++a)
		{
			U32 currentWidth = surface.GetWidth();
			U32 currentHeight = surface.GetHeight();
			U16 currentDepth = surface.GetDepth();
			for (U16 mip = 0; mip < surface.GetMipCount(); ++mip)
			{
				const U32 rowSize = surface.GetRowByteSize(mip);
				for (U16 d = 0; d < currentDepth; ++d)
				{
					for (U32 y = 0; y < currentHeight; ++y)
					{
						// Only place with row alignment, everything above have slice alignment
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
	}

	void ConvertToCubemap(const GFX::Surface& surface, GFX::Surface& cubemap, U32 cores,
		Math::FilterType filter, float filterCoeff, U32 windowSize, bool noAlpha, bool fp16) noexcept
	{
		U8* cubemapBuffer = cubemap.GetBuffer();
		const U8* hdriBuffer = surface.GetBuffer();

		const U32 hdriRowSize = surface.GetRowByteSize();
		const U32 rowSize = cubemap.GetRowByteSize();
		const U64 sliceSize = cubemap.GetSliceByteSize();
		const U8 pixelSize = cubemap.GetPixelSize();

		const S32 halfWindow = Utils::SafeCast<S32>(windowSize) >> 1;
		std::vector<float> filterCoeffs = Math::GetFilterCoefficients(filter, windowSize, filterCoeff);

		auto processCubemap = [&surface, &cubemap, &filterCoeffs, halfWindow, hdriBuffer, hdriRowSize, rowSize, sliceSize, pixelSize, filter, windowSize, noAlpha, fp16](U8* cubemapBuffer, U16 startFace, U16 endFace)
			{
				for (U16 a = startFace; a < endFace; ++a)
				{
					const Math::CubemapFaceTraversalDesc& faceDesc = Math::CUBEMAP_FACES_INFO.at(a);
					const Vector faceStart = Math::XMLoadFloat3(&faceDesc.StartPos);
					const Vector xDir = Math::XMLoadFloat3(&faceDesc.DirX);
					const Vector yDir = Math::XMLoadFloat3(&faceDesc.DirY);

					for (U32 y = 0; y < cubemap.GetHeight(); ++y)
					{
						const Vector yScale = Math::XMVectorReplicate((static_cast<float>(y) + 0.5f) / static_cast<float>(cubemap.GetHeight()));
						const Vector rowPos = Math::XMVectorMultiplyAdd(yDir, yScale, faceStart);

						for (U32 x = 0; x < cubemap.GetWidth(); ++x)
						{
							const Vector xScale = Math::XMVectorReplicate((static_cast<float>(x) + 0.5f) / static_cast<float>(cubemap.GetWidth()));
							const Vector direction = Math::XMVector3Normalize(Math::XMVectorMultiplyAdd(xDir, xScale, rowPos));

							const float dirY = Math::XMVectorGetY(direction);
							const float dirZ = Math::XMVectorGetZ(direction);

							const float azimuthAngle = std::atan2f(Math::XMVectorGetX(direction), dirZ) + Math::PI; // Longitude
							const float polarAngle = std::atanf(dirY / Math::XMVectorGetX(Math::XMVector2Length(Math::XMVectorSetY(direction, dirZ)))) + static_cast<float>(M_PI_2); // Lattitude

							const float hdriX = (1.0f - azimuthAngle / Math::PI2) * static_cast<float>(surface.GetWidth());
							const float hdriY = (1.0f - polarAngle / Math::PI) * static_cast<float>(surface.GetHeight());

							Float3 hdriPixel = {};
							switch (filter)
							{
							default:
								ZE_ENUM_UNHANDLED();
							case Math::FilterType::Box:
							{
								const U32 hdriXIdx = std::clamp(static_cast<U32>(hdriX), 0U, surface.GetWidth() - 1);
								const U32 hdriYIdx = std::clamp(static_cast<U32>(hdriY), 0U, surface.GetHeight() - 1);
								hdriPixel = *reinterpret_cast<const Float3*>(hdriBuffer + hdriYIdx * hdriRowSize + hdriXIdx * sizeof(Float3));
								break;
							}
							case Math::FilterType::Bilinear:
							{
								// Factor gives the contribution of the next column, while the contribution of intX is 1 - factor
								float intX, intY;
								float factorX = std::modf(hdriX - 0.5f, &intX);
								float factorY = std::modf(hdriY - 0.5f, &intY);

								U32 lowYIdx = static_cast<U32>(intY);
								U32 lowXIdx = static_cast<U32>(intX);

								U32 highXIdx;
								if (factorX < 0.0f)
									highXIdx = surface.GetWidth() - 1;
								else if (lowXIdx == surface.GetWidth() - 1)
									highXIdx = 0;
								else
									highXIdx = lowXIdx + 1;

								U32 highYIdx;
								if (factorY < 0.0f)
									highYIdx = surface.GetHeight() - 1;
								else if (lowYIdx == surface.GetHeight() - 1)
									highYIdx = 0;
								else
									highYIdx = lowYIdx + 1;

								std::vector<Float4> samples;
								samples.resize(4);
								Float3 temp = *reinterpret_cast<const Float3*>(hdriBuffer + lowYIdx * hdriRowSize + lowXIdx * sizeof(Float3));
								samples.at(0) = { temp.x, temp.y, temp.z, 0.0f };
								temp = *reinterpret_cast<const Float3*>(hdriBuffer + highYIdx * hdriRowSize + lowXIdx * sizeof(Float3));
								samples.at(1) = { temp.x, temp.y, temp.z, 0.0f };
								temp = *reinterpret_cast<const Float3*>(hdriBuffer + lowYIdx * hdriRowSize + highXIdx * sizeof(Float3));
								samples.at(2) = { temp.x, temp.y, temp.z, 0.0f };
								temp = *reinterpret_cast<const Float3*>(hdriBuffer + highYIdx * hdriRowSize + highXIdx * sizeof(Float3));
								samples.at(3) = { temp.x, temp.y, temp.z, 0.0f };

								const Vector interpolated = Math::ApplyFilter(Math::FilterType::Bilinear, samples, std::abs(factorX), std::abs(factorY));
								Math::XMStoreFloat3(&hdriPixel, interpolated);
								break;
							}
							case Math::FilterType::GammaAverage:
							case Math::FilterType::Kaiser:
							case Math::FilterType::Lanczos:
							case Math::FilterType::Gauss:
							{
								S32 baseYIdx = static_cast<S32>(hdriX);
								S32 baseXIdx = static_cast<S32>(hdriY);

								std::vector<Float4> samples;
								samples.reserve(static_cast<U64>(windowSize) * windowSize);
								for (S32 dy = -halfWindow - (windowSize & 1); dy < halfWindow; ++dy)
								{
									S32 pointY = baseYIdx + dy;
									if (pointY < 0)
										pointY += surface.GetHeight();
									else if (static_cast<U32>(pointY) >= surface.GetHeight())
										pointY -= surface.GetHeight();

									for (S32 dx = -halfWindow - (windowSize & 1); dx < halfWindow; ++dx)
									{
										S32 pointX = baseXIdx + dx;
										if (pointX < 0)
											pointX += surface.GetWidth();
										else if (static_cast<U32>(pointX) >= surface.GetWidth())
											pointX -= surface.GetWidth();

										const Float3 sample = *reinterpret_cast<const Float3*>(hdriBuffer + pointY * hdriRowSize + pointX * sizeof(Float3));
										samples.emplace_back(sample.x, sample.y, sample.z, 0.0f);
									}
								}

								const Vector interpolated = Math::ApplyFilter(filter, samples, 0.0f, 0.0f, &filterCoeffs);
								Math::XMStoreFloat3(&hdriPixel, interpolated);
								break;
							}
							case Math::FilterType::BicubicSharp:
							case Math::FilterType::BicubicSmooth:
							{
								// Base integer pixel
								float intX, intY;
								float factorX = std::modf(hdriX, &intX);
								float factorY = std::modf(hdriY, &intY);

								U32 baseYIdx = static_cast<U32>(intY);
								U32 baseXIdx = static_cast<U32>(intX);

								std::vector<Float4> samples;
								samples.reserve(16);
								for (S32 dy = -1; dy <= 2; ++dy)
								{
									S32 pointY = baseYIdx + dy;
									if (pointY < 0)
										pointY += surface.GetHeight();
									else if (static_cast<U32>(pointY) >= surface.GetHeight())
										pointY -= surface.GetHeight();

									for (S32 dx = -1; dx <= 2; ++dx)
									{
										S32 pointX = baseXIdx + dx;
										if (pointX < 0)
											pointX += surface.GetWidth();
										else if (static_cast<U32>(pointX) >= surface.GetWidth())
											pointX -= surface.GetWidth();

										const Float3 sample = *reinterpret_cast<const Float3*>(hdriBuffer + pointY * hdriRowSize + pointX * sizeof(Float3));
										samples.emplace_back(sample.x, sample.y, sample.z, 0.0f);
									}
								}

								const Vector interpolated = Math::ApplyFilter(filter, samples, factorX, factorY);
								Math::XMStoreFloat3(&hdriPixel, interpolated);
								break;
							}
							}

							const U64 cubemapOffset = Utils::SafeCast<U64>(y) * rowSize + Utils::SafeCast<U64>(x) * pixelSize;
							if (fp16)
							{
								*reinterpret_cast<U16*>(cubemapBuffer + cubemapOffset) = Math::FP16::EncodeFloat16(hdriPixel.x);
								*reinterpret_cast<U16*>(cubemapBuffer + cubemapOffset + 2) = Math::FP16::EncodeFloat16(hdriPixel.y);
								*reinterpret_cast<U16*>(cubemapBuffer + cubemapOffset + 4) = Math::FP16::EncodeFloat16(hdriPixel.z);
								*reinterpret_cast<U16*>(cubemapBuffer + cubemapOffset + 6) = Math::FP16::EncodeFloat16(1.0f);
							}
							else if (noAlpha)
								*reinterpret_cast<Float3*>(cubemapBuffer + cubemapOffset) = hdriPixel;
							else
								*reinterpret_cast<Float4*>(cubemapBuffer + cubemapOffset) = { hdriPixel.x, hdriPixel.y, hdriPixel.z, 1.0f };
						}
					}
					cubemapBuffer += sliceSize;
				}
			};

		if (cores > 1)
		{
			cores = std::clamp(cores, 2U, 6U);
			U16 jobSize = Utils::SafeCast<U16>(6 / cores);
			--cores;
			std::vector<std::thread> workers;
			workers.reserve(cores);
			for (U16 i = 0; i < cores; ++i)
			{
				U16 jobOffset = i * jobSize;
				workers.emplace_back(processCubemap, cubemapBuffer, jobOffset, static_cast<U16>(jobOffset + jobSize));
				cubemapBuffer += jobSize * sliceSize;
			}
			processCubemap(cubemapBuffer, Utils::SafeCast<U16>(jobSize * cores), 6);
			for (auto& worker : workers)
				worker.join();
		}
		else
			processCubemap(cubemapBuffer, 0, 6);
	}
}
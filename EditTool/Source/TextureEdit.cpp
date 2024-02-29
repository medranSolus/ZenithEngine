#include "TextureEdit.h"
#include "GFX/Surface.h"

void TextureEdit::NoAlpha(const std::string& source, const std::string& destination)
{
	ZE::GFX::Surface surface;
	if (surface.Load(source))
	{
		if (ZE::Utils::GetChannelCount(surface.GetFormat()) >= 4)
		{
			const U8 channelSize = ZE::Utils::GetChannelSize(surface.GetFormat());
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
								buffer[channelSize * 3] = 255;
								buffer += surface.GetPixelSize();
							}
							buffer = reinterpret_cast<U8*>(ZE::Math::AlignUp(reinterpret_cast<U64>(buffer), static_cast<U64>(ZE::GFX::Surface::ROW_PITCH_ALIGNMENT)));
						}
						buffer = reinterpret_cast<U8*>(ZE::Math::AlignUp(reinterpret_cast<U64>(buffer), static_cast<U64>(ZE::GFX::Surface::SLICE_PITCH_ALIGNMENT)));
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
			if (surface.Save(destination))
			{
				std::string msg = "Reseted alpha channel of \"" + source + "\" and saved to ";
				if (source == destination)
					ZE::Logger::InfoNoFile(msg + "same file.");
				else
					ZE::Logger::InfoNoFile(msg + "\"" + destination + "\".");
			}
		}
		else
			ZE::Logger::Warning("No alpha channel in texture!");
	}
}

void TextureEdit::FlipY(const std::string& source, const std::string& destination)
{
	ZE::GFX::Surface surface;
	if (surface.Load(source))
	{
		if (ZE::Utils::GetChannelCount(surface.GetFormat()) >= 2)
		{
			const U8 channelSize = ZE::Utils::GetChannelSize(surface.GetFormat());
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
								buffer[channelSize] = 255 - buffer[channelSize];
								buffer += surface.GetPixelSize();
							}
							buffer = reinterpret_cast<U8*>(ZE::Math::AlignUp(reinterpret_cast<U64>(buffer), static_cast<U64>(ZE::GFX::Surface::ROW_PITCH_ALIGNMENT)));
						}
						buffer = reinterpret_cast<U8*>(ZE::Math::AlignUp(reinterpret_cast<U64>(buffer), static_cast<U64>(ZE::GFX::Surface::SLICE_PITCH_ALIGNMENT)));
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
			if (surface.Save(destination))
			{
				std::string msg = "Fliped Y (G channel) of \"" + source + "\" and saved to ";
				if (source == destination)
					ZE::Logger::InfoNoFile(msg + "same file.");
				else
					ZE::Logger::InfoNoFile(msg + "\"" + destination + "\".");
			}
		}
		else
			ZE::Logger::Warning("Not enough channels in texture!");
	}
}
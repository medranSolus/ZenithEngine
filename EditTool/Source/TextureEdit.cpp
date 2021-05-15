#include "TextureEdit.h"
#include "GFX/Surface.h"
#include "Logger.h"

void TextureEdit::NoAlpha(const std::string& source, const std::string& destination)
{
	ZE::GFX::Surface surface(source);
	ZE::Pixel* buffer = surface.GetBuffer();
	for (U64 i = 0; i < surface.GetSize(); ++i)
		buffer[i].Alpha = 255;
	surface.Save(destination);
	std::string msg = "Reseted alpha channel of \"" + source + "\" and saved to ";
	if (source == destination)
		ZE::Logger::InfoNoFile(msg + "same file.");
	else
		ZE::Logger::InfoNoFile(msg + "\"" + destination + "\".");
}

void TextureEdit::FlipY(const std::string& source, const std::string& destination)
{
	ZE::GFX::Surface surface(source);
	ZE::Pixel* buffer = surface.GetBuffer();
	for (U64 i = 0; i < surface.GetSize(); ++i)
		buffer[i].Green = 255 - buffer[i].Green;
	surface.Save(destination);
	std::string msg = "Fliped Y (G channel) of \"" + source + "\" and saved to ";
	if (source == destination)
		ZE::Logger::InfoNoFile(msg + "same file.");
	else
		ZE::Logger::InfoNoFile(msg + "\"" + destination + "\".");
}
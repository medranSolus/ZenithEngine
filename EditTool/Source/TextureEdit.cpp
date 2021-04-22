#include "TextureEdit.h"
#include "GFX/Surface.h"
#include "Logger.h"

void TextureEdit::NoAlpha(const std::string& source, const std::string& destination)
{
	GFX::Surface surface(source);
	Pixel* buffer = surface.GetBuffer();
	for (U64 i = 0; i < surface.GetSize(); ++i)
		buffer[i].Alpha = 255;
	surface.Save(destination);
	std::string msg = "Reseted alpha channel of \"" + source + "\" and saved to ";
	if (source == destination)
		Logger::InfoNoFile(msg + "same file.");
	else
		Logger::InfoNoFile(msg + "\"" + destination + "\".");
}

void TextureEdit::FlipY(const std::string& source, const std::string& destination)
{
	GFX::Surface surface(source);
	Pixel* buffer = surface.GetBuffer();
	for (U64 i = 0; i < surface.GetSize(); ++i)
		buffer[i].Green = 255 - buffer[i].Green;
	surface.Save(destination);
	std::string msg = "Fliped Y (G channel) of \"" + source + "\" and saved to ";
	if (source == destination)
		Logger::InfoNoFile(msg + "same file.");
	else
		Logger::InfoNoFile(msg + "\"" + destination + "\".");
}
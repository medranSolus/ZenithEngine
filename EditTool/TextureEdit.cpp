#include "TextureEdit.h"
#include "Surface.h"
#include "Logger.h"

void TextureEdit::NoAlpha(const std::string& source, const std::string& destination)
{
	GFX::Surface surface(source);
	GFX::Surface::Pixel* buffer = surface.GetBuffer();
	for (size_t i = 0; i < surface.GetSize(); ++i)
		buffer[i].SetA(255);
	surface.Save(destination);
	if (source == destination)
		Logger::InfoNoFile("Reseted alpha channel of \"" + source + "\" and saved to same file.");
	else
		Logger::InfoNoFile("Reseted alpha channel of \"" + source + "\" and saved to \"" + destination + "\".");
}

void TextureEdit::FlipY(const std::string& source, const std::string& destination)
{
	GFX::Surface surface(source);
	GFX::Surface::Pixel* buffer = surface.GetBuffer();
	for (size_t i = 0; i < surface.GetSize(); ++i)
		buffer[i].SetG(255 - buffer[i].GetG());
	surface.Save(destination);
	if (source == destination)
		Logger::InfoNoFile("Fliped Y (G channel) of \"" + source + "\" and saved to same file.");
	else
		Logger::InfoNoFile("Fliped Y (G channel) of \"" + source + "\" and saved to \"" + destination + "\".");
}
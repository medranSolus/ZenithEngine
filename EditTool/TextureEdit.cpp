#include "TextureEdit.h"
#include "Surface.h"

void TextureEdit::NoAlpha(const std::string& source, const std::string& destination)
{
	GFX::Surface surface(source);
	GFX::Surface::Pixel* buffer = surface.GetBuffer();
	for (size_t i = 0; i < surface.GetSize(); ++i)
		buffer[i].SetA(255);
	surface.Save(destination);
}
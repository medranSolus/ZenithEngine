#pragma once
#include <string>

class TextureEdit
{
public:
	TextureEdit() = delete;

	static void NoAlpha(const std::string& source, const std::string& destination);
	static void FlipY(const std::string& source, const std::string& destination);
};
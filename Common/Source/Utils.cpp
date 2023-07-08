#include "Utils.h"
#include <iomanip>
#include <cstdlib>

namespace ZE::Utils
{
	std::wstring ToUTF16(std::string_view s) noexcept
	{
		constexpr const wchar_t* ERROR_STR = L"[Not UTF-8 string]";

		std::wstring str;
		str.reserve(s.size());
		U64 i = 0;
		while (i < s.size())
		{
			U32 unicode;
			U64 remaining = 0;
			U8 c = s[i++];

			if (c <= 0x7F)
				unicode = c;
			else if (c <= 0xBF)
				return ERROR_STR;
			else if (c <= 0xDF)
			{
				unicode = c & 0x1F;
				remaining = 1;
			}
			else if (c <= 0xEF)
			{
				unicode = c & 0x0F;
				remaining = 2;
			}
			else if (c <= 0xF7)
			{
				unicode = c & 0x07;
				remaining = 3;
			}
			else
				return ERROR_STR;
			// Encode multiple characters on 6 bits
			while (remaining--)
			{
				if (i == s.size())
					return ERROR_STR;
				c = s[i++];
				if (c < 0x80 || c > 0xBF)
					return ERROR_STR;
				unicode <<= 6;
				unicode += c & 0x3F;
			}
			if ((unicode >= 0xD800 && unicode <= 0xDFFF) || unicode > 0x10FFFF)
				return ERROR_STR;

			// Encode unicode character as UTF-16
			if (unicode <= 0xFFFF)
				str.push_back(static_cast<wchar_t>(unicode));
			else
			{
				unicode -= 0x10000;
				str.push_back(static_cast<wchar_t>((unicode >> 10) + 0xD800));
				str.push_back(static_cast<wchar_t>((unicode & 0x3FF) + 0xDC00));
			}
		}
		return str;
	}

	std::string ToUTF8(std::wstring_view s) noexcept
	{
		constexpr const char* ERROR_STR = "[Not UTF-16 string]";

		std::string str;
		str.reserve(s.size());
		for (U64 i = 0; i < s.size(); ++i)
		{
			// Restore unicode character
			U32 unicode = s[i];
			if (unicode >= 0xD800 && unicode <= 0xDFFF)
				unicode = ((unicode - 0xD800) << 10) + s[++i] + 0x2400;

			if (unicode <= 0x7F)
				str.push_back(static_cast<char>(unicode));
			else if (unicode <= 0x7FF)
			{
				str.push_back(static_cast<char>(0xC0 + (unicode >> 6)));
				str.push_back(static_cast<char>(0x80 + (unicode & 0x3F)));
			}
			else if (unicode <= 0xFFFF)
			{
				// 3 bytes
				str.push_back(static_cast<char>(0xE0 + (unicode >> 12)));
				str.push_back(static_cast<char>(0x80 + ((unicode >> 6) & 0x3F)));
				str.push_back(static_cast<char>(0x80 + (unicode & 0x3F)));
			}
			else if (unicode <= 0x10FFFF)
			{
				//4 bytes
				str.push_back(static_cast<char>(0xF0 + (unicode >> 18)));
				str.push_back(static_cast<char>(0x80 + ((unicode >> 12) & 0x3F)));
				str.push_back(static_cast<char>(0x80 + ((unicode >> 6) & 0x3F)));
				str.push_back(static_cast<char>(0x80 + (unicode & 0x3F)));
			}
			else
				return ERROR_STR;
		}
		return str;
	}

	std::vector<std::string> ParseQuoted(const std::string& input) noexcept
	{
		std::istringstream stream(input);
		std::vector<std::string> output;
		std::string element;
		while (stream >> std::quoted(element))
			output.emplace_back(std::move(element));
		return output;
	}

	std::deque<std::string_view> SplitString(std::string_view input, std::string_view delimiter) noexcept
	{
		size_t pos = 0, offset = 0, step = delimiter.size();
		std::deque<std::string_view> output;
		while ((pos = input.find(delimiter, offset)) != std::string::npos)
		{
			if (pos != offset)
			{
				output.emplace_back(input.substr(offset, pos - offset));
				// Remove trailing spaces
				while (output.back().at(output.back().size() - 1) == ' ')
					output.back() = output.back().substr(0, output.back().size() - 1);
			}
			offset = pos + step;
		}
		output.emplace_back(input.substr(offset, input.size() - offset));
		// Remove trailing spaces
		while (output.back().at(output.back().size() - 1) == ' ')
			output.back() = output.back().substr(0, output.back().size() - 1);
		return output;
	}

	void* AlignedAlloc(U64 size, U64 alignment) noexcept
	{
		ZE_ASSERT(size != 0, "Invalid size of allocation!");
		ZE_ASSERT(Math::IsPower2(alignment), "Alignment must be power of 2!");

		size = Math::AlignUp(size, alignment);
#if _ZE_COMPILER_MSVC
		return _aligned_malloc(size, alignment);
#else
		return std::aligned_alloc(alignment, size);
#endif
	}

	void* AlignedRealloc(void* ptr, U64 newSize, U64 oldSize, U64 alignment) noexcept
	{
		ZE_ASSERT(ptr, "Invalid allocation!");
		ZE_ASSERT(newSize != 0 && oldSize != 0, "Invalid size of allocation!");
		ZE_ASSERT(Math::IsPower2(alignment), "Alignment must be power of 2!");

#if _ZE_COMPILER_MSVC
		return _aligned_realloc(ptr, newSize, alignment);
#else
		if (alignment <= alignof(std::max_align_t))
			return realloc(ptr, newSize);

		void* newBlock = AlignedAlloc(newSize, alignment);
		if (newBlock)
			memcpy(newBlock, ptr, oldSize);

		AlignedFree(ptr);
		return newBlock;
#endif
	}

	void AlignedFree(void* ptr) noexcept
	{
		ZE_ASSERT(ptr, "Invalid allocation!");

#if _ZE_COMPILER_MSVC
		_aligned_free(ptr);
#else
		free(ptr);
#endif
	}
}
#pragma once
#include "BasicTypes.h"

namespace ZE::IO::WAV
{
	// https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
	
	// Information about format chunk size
	typedef U32 FormatChunkSize;
	// Possible sizes of the format chunk header
	enum class FormatHeaderSize : FormatChunkSize
	{
		Legacy = 16, // Legacy PCM file
		Default = 18, // Standard PCM-like format
		Extended = 40, // Extended format with additional information
	};
	ZE_ENUM_OPERATORS(FormatHeaderSize, FormatChunkSize);

	// Information about sample format type
	typedef U16 FormatTags;
	// Possible sizes of the format chunk header
	enum class FormatTag : FormatTags
	{
		PCM = 1,
		Float = 3,
		Extended = 0xFFFE,
	};
	ZE_ENUM_OPERATORS(FormatTag, FormatTags);

	// Information about format chunk size
	typedef U16 FormatExtensionSize;
	// Possible sizes of the format chunk header
	enum class FormatExtensionHeaderSize : FormatExtensionSize
	{
		None = 0, // Standard PCM-like format
		Extensible = 22,
	};
	ZE_ENUM_OPERATORS(FormatExtensionHeaderSize, FormatExtensionSize);

#pragma pack(push, 1)
	// Main header of the WAV file format
	struct RiffChunkHeader
	{
		U32 FileTypeMagicNumber; // 'RIFF'
		U32 FileSize; // Minus 8 bytes of the entire file
		U32 FileFormatMagicNumber; // 'WAVE'
	};

	// Basic information about the format of the WAV file
	struct FormatChunkHeader
	{
		U32 MagicNumber; // 'fmt '
		FormatChunkSize Size;
		FormatTags AudioFormat;
		U16 NumChannels;
		U32 SampleRate; // Hz
		U32 ByteRate; // SampleRate * BlockAlign
		U16 BlockAlign; // NumChannels * ceil(BitsPerSample / 8)
		U16 BitsPerSample; // Aligned to 8 bits
	};

	// Current extended format of the WAV file
	struct FormatExChunkHeader
	{
		FormatChunkHeader Format;
		FormatExtensionSize ExtensionSize;
	};

	// Current extended format of the WAV file
	struct FormatExtensionChunkHeader
	{
		FormatExChunkHeader FormatEx;
		U16 BitsPerSample; // Real number of valid bits in the sample
		U32 ChannelMask;
		U8 SubformatGUID[16];
	};

	// Header indicating data section
	struct DataChunkHeader
	{
		U32 MagicNumber; // 'data'
		U32 SampleDataSize;
	};
#pragma pack(pop)

	static_assert(sizeof(RiffChunkHeader) == 12, "Incorret size of WAV file header!");
	static_assert(sizeof(FormatChunkHeader) == 24, "Incorret size of WAV format header!");
	static_assert(sizeof(FormatExChunkHeader) == 26, "Incorret size of WAV extended format header!");
	static_assert(sizeof(FormatExtensionChunkHeader) == 48, "Incorret size of WAV format extension header!");
	static_assert(sizeof(DataChunkHeader) == 8, "Incorret size of WAV data chunk header!");
}
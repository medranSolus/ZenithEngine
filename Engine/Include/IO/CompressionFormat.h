#pragma once

namespace ZE::IO
{
	// Type of compression that assets is saved with
	enum class CompressionFormat : U8
	{
		None,
		ZLib,
		Bzip2,
	};
}
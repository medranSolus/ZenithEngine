#pragma once
#include "BasicTypes.h"

namespace ZE::AHI
{
	// Possible supported audio APIs
	enum class ApiType : U8 { None, OpenAL, XAudio2 };
}
namespace ZE
{
	// Possible supported audio APIs
	typedef AHI::ApiType AudioApiType;
}
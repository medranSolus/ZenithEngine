#pragma once
// Headers needed for XAudio2
#include "Platform/WinAPI/ComPtr.h"
ZE_WARNING_PUSH
#include "xapobase.h"
#include "xapofx.h"
#include "xaudio2.h"
#include "xaudio2fx.h"
namespace XDSP
{
	using namespace ZE::External;
}
#include "xdsp.h"
namespace DirectX
{
	typedef Float3 XMFLOAT3;
}
#include "x3daudio.h"
ZE_WARNING_POP

namespace ZE::AHI::XAudio2
{
	// Enable ComPtr for all XAudio2 namespace
	using Platform::WinAPI::ComPtr;
}
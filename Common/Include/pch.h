#pragma once
/*
* Precompiled headers listed here don't have to be included into .cpp files as they are added via buildsystem.
* Note that they must still be included into header files that require them!
*
* Before adding new header specify all it's includes recursively in appearing order with following style.
* Also changes to than files should be considered with updating include lists.
* When in following includes appear same header it's content don't have to be listed.
* All listed headers should have global directory specyfication.
*
* REMINDER: After changing that list check other project's PCH to avoid include duplication!
*/

/*
********* BasicTypes.h (defined by CmdParser.h)
********* intrin.h / x86intrin.h + cpuid.h
******* Intrinsics.h
*********** atomic
*********** cstdint
********* BasicTypes.h
********* shared_mutex
********* string
******* Logger.h
***** Macros.h
***** memory
*** Ptr.h
*** cmath
*** cstddef
*** cfloat
*** DirectXMath.h
*** DirectXCollision.h
* Types.h
* deque
* string_view
* unordered_map
* vector
*/
#include "CmdParser.h"

/*
* Types.h (defined by CmdParser.h)
* utility
*/
#include "ColorF3.h"

/*
***** Types.h (defined by CmdParser.h)
***** utility
*** ColorF4.h
*** array
*** random
* MathExt.h
*/
#include "MathLight.h"

/*
* MathExt.h (defined by MathLight.h)
* cstring
* limits
*/
#include "MathFP16.h"

/*
*** Types.h (defined by CmdParser.h)
*** bitset
*** map
*** shared_mutex
*** string
* Perf.h
*/
#include "PerfGuard.h"

/*
* Types.h (defined by CmdParser.h)
*/
#include "Pixel.h"

/*
* Types.h (defined by CmdParser.h)
* type_traits
* cstdlib
* cstring
*/
#include "Table.h"

/*
* Macros.h (defined by CmdParser.h)
* chrono
*/
#include "Timer.h"

/*
***** Macros.h (defined by CmdParser.h)
***** shared_mutex
*** LockGuard.h
*** memory
* Allocator/BlockingQueue.h
*** BasicTypes.h (defined by CmdParser.h)
*** type_traits
*** vector
* Allocator/FixedPool.h
*** BasicTypes.h (defined by CmdParser.h)
*** future
*** memory
* Task.h
* array
* condition_variable
* functional
* thread
*/
#include "ThreadPool.h"

/*
* Types.h (defined by CmdParser.h)
*** BasicTypes.h (defined by CmdParser.h)
* PixelFormat.h
* bit
* deque
* limits
* string
* vector
*/
#include "Utils.h"

/*
*** BasicTypes.h (defined by CmdParser.h)
*** vector
* Allocator/Pool.h
* Intrinsics.h (defined by CmdParser.h)
* bitset
*/
#include "Allocator/ChunkedTLSF.h"

/*
***** Macros.h (defined by CmdParser.h)
*** DDS/PixelFormatDDS.h
* DDS/Header.h
***** BasicTypes.h (defined by CmdParser.h)
*** DDS/FormatDDS.h
*** Macros.h (defined by CmdParser.h)
* DDS/HeaderDXT10.h
* PixelFormat.h (defined by Utils.h)
*/
#include "DDS/Utils.h"

/*
*** Types.h (defined by CmdParser.h)
*** exception
*** string
* Exception/BasicException.h
*/
#include "Exception/GenericException.h"

/*
* PixelFormat.h (defined by Utils.h)
* filesystem
* utility
* vector
*/
#include "GFX/Surface.h"

/*
* Standard headers, remove if appears above
*/
#include <algorithm>
#include <cinttypes>
#include <sstream>
#include <fstream>

/*
* Platform specific headers
*/
#if _ZE_PLATFORM_WINDOWS
/*
* Exception/BasicException.h (defined by Exception/GenericException.h)
*** sdkddkver.h
*** Windows.h
* Platform/WinAPI/WinAPI.h
*/
#	include "Platform/WinAPI/WinApiException.h"

/*
* Platform/WinAPI/WinAPI.h (defined by Platform/WinAPI/WinApiException.h)
* Utils.h (defined by platform agnostic headers)
*/
#	include "Platform/WinAPI/Perf.h"
#endif
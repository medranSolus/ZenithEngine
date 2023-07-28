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
*********** cassert
********* Macros.h
******* Ptr.h
******* atomic
******* cstdint
***** BasicTypes.h
***** intrin.h/x86intrin.h + cpuid.h
*** Intrinsics.h
*** cmath
*** cstddef
*** cfloat
*** DirectXMath.h
*** DirectXCollision.h
* Types.h
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
* string
*/
#include "Logger.h"

/*
*** Types.h (defined by CmdParser.h)
*** utility
* ColorF4.h
* random
*/
#include "MathExt.h"

/*
* Types.h (defined by CmdParser.h)
* string
* map
*/
#include "Perf.h"

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
*** Types.h (defined by CmdParser.h)
* PixelFormat.h
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
*** Types.h (defined by CmdParser.h)
*** exception
*** string
* Exception/BasicException.h
*/
#include "Exception/GenericException.h"

/*
*** Types.h (defined by CmdParser.h)
* Pixel.h
*** PixelFormat.h (defined by Utils.h)
*** DirectXTex.h
* GFX/DX.h
* utility
* vector
*/
#include "GFX/Surface.h"

/*
* Standard headers, remove if appears above
*/
#include <algorithm>
#include <sstream>
#include <filesystem>
#include <fstream>

/*
* Platform specific headers
*/
#if _ZE_PLATFORM_WINDOWS
/*
* Exception/GenericException.h (defined by platform agnostic headers)
*** Exception/BasicException.h (defined by Exception/ImageException.h)
***** sdkddkver.h
***** Windows.h
*** Platform/WinAPI/WinAPI.h
* Platform/WinAPI/WinApiException.h
*/
#	include "Platform/WinAPI/DirectXTexException.h"
#endif
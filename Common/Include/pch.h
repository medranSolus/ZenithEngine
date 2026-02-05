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
* atomic
* expected
* system_error
* cstdint
*/
#include "BasicTypes.h"

/*
* Types.h
* deque
* string_view
* unordered_map
*/
#include "CmdParser.h"

/*
* Types.h (defined by CmdParser.h)
* utility
*/
#include "ColorF3.h"

/*
* Types.h (defined by CmdParser.h)
* utility
*/
#include "ColorF4.h"

/*
* BasicTypes.h
* intrin.h / x86intrin.h + cpuid.h
*/
#include "Intrinsics.h"

/*
* Macros.h (defined by CmdParser.h)
*/
#include "LockGuard.h"

/*
* BasicTypes.h
* functional
* iostream
* shared_mutex
* string_view
*/
#include "Logger.h"

/*
* Logger.h
* Intrinsics.h
*/
#include "Macros.h"

/*
* ColorF4.h
* array
* random
* cfloat
* cmath
*/
#include "MathExt.h"

/*
* BasicTypes.h
* limits
* type_traits
* cstring
*/
#include "MathFP16.h"

/*
* Types.h
*/
#include "MathLight.h"

/*
* BasicTypes.h
* bitset
* map
* shared_mutex
* string
* vector
* Platform/X/Perf.h
*/
#include "Perf.h"

/*
* Perf.h
*/
#include "PerfGuard.h"

/*
* BasicTypes.h
*/
#include "Pixel.h"

/*
* BasicTypes.h
*/
#include "PixelFormat.h"

/*
* Macros.h
* cstdlib
*/
#include "Ptr.h"

/*
* Utils.h
* type_traits
* cstdlib
* cstring
*/
#include "Table.h"

/*
* BasicTypes.h
* future
* memory
*/
#include "Task.h"

/*
* Allocator/BlockingQueue.h
* Allocator/FixedPool.h
* Task.h
* array
* condition_variable
* thread
* vector
*/
#include "ThreadPool.h"

/*
* Macros.h
* chrono
*/
#include "Timer.h"

/*
* Ptr.h
* DirectXMath.h
* DirectXCollision.h
*/
#include "Types.h"

/*
* Types.h
* PixelFormat.h
* bit
* deque
* limits
* string
* vector
*/
#include "Utils.h"

/*
* LockGuard.h
* memory
*/
#include "Allocator/BlockingQueue.h"

/*
* Allocator/Pool.h
* Intrinsics.h
* bitset
*/
#include "Allocator/ChunkedTLSF.h"

/*
* BasicTypes.h
* memory
* type_traits
* vector
*/
#include "Allocator/FixedPool.h"

/**/
#include "Allocator/OperatorNew.h"

/*
* BasicTypes.h
* vector
*/
#include "Allocator/Pool.h"

/*
* PixelFormat.h
* memory
*/
#include "GFX/Surface.h"

/*
* BasicTypes.h
* string
*/
#include "IO/DDS/FileResult.h"

/*
* BasicTypes.h
*/
#include "IO/DDS/FormatDDS.h"

/*
* IO/DDS/PixelFormatDDS.h
*/
#include "IO/DDS/Header.h"

/*
* IO/DDS/FormatDDS.h
* Macros.h
*/
#include "IO/DDS/HeaderDXT10.h"

/*
* Macros.h
*/
#include "IO/DDS/PixelFormatDDS.h"

/*
* IO/File.h
* IO/DDS/Header.h
* IO/DDS/HeaderDXT10.h
*/
#include "IO/DDS/Utils.h"

/*
* IO/FileFlags.h
* cstdio
* Platform/X/File.h
*/
#include "IO/File.h"

/*
* Macros.h
*/
#include "IO/FileFlags.h"

/*
* Standard headers, remove if appears above
*/
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cinttypes>

/*
* Platform specific headers
*/
#if _ZE_PLATFORM_WINDOWS
/*
* BasicTypes.h
* WinAPI.h
*/
#	include "Platform/WinAPI/Error.h"

/*
* IO/FileFlags.h
* Platform/WinAPI/Error.h
* Task.h
* Platform/WinAPI/WinAPI.h
*/
#	include "Platform/WinAPI/File.h"

/*
* Platform/WinAPI/WinAPI.h
* Utils.h
*/
#	include "Platform/WinAPI/Perf.h"

/*
* sdkddkver.h
* Windows.h
*/
#	include "Platform/WinAPI/WinAPI.h"
#endif
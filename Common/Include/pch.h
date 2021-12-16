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
***** cassert
*** Macros.h
*** atomic
*** cmath
*** cstddef
*** cstdint
*** cfloat
*** intrin.h
*** DirectXMath.h
*** DirectXCollision.h
* Types.h
* utility
*/
#include "ColorF3.h"

/*
* string
*/
#include "Logger.h"

/*
*** Types.h (defined by ColorF3.h)
*** utility
* ColorF4.h
* random
*/
#include "MathExt.h"

/*
* Types.h (defined by ColorF3.h)
* string
* map
*/
#include "Perf.h"

/*
* Types.h (defined by ColorF3.h)
*** Types.h (defined by ColorF3.h)
* PixelFormat.h
* string
* vector
* deque
*/
#include "Utils.h"

/*
*** Types.h (defined by ColorF3.h)
*** exception
*** string
* Exception/BasicException.h
*/
#include "Exception/GenericException.h"

/*
*** Types.h (defined by ColorF3.h)
* Pixel.h
*** PixelFormat.h (defined by Utils.h)
*** DirectXTex.h
* GFX/DX.h
* functional
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
#ifdef _ZE_PLATFORM_WINDOWS
/*
* Exception/GenericException.h (defined by platform agnostic headers)
*** Exception/BasicException.h (defined by Exception/ImageException.h)
***** sdkddkver.h
***** Windows.h
*** Platform/WinAPI/WinAPI.h
* Platform/WinAPI/WinApiException.h
*/
#include "Platform/WinAPI/DirectXTexException.h"
#endif
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
***** atomic
***** cmath
***** cstddef
***** cstdint
***** cfloat
***** cassert
***** intrin.h
***** DirectXMath.h
*** Types.h
* ColorF4.h
* utility
* random
*/
#include "MathExt.h"

/*
* Types.h (defined by MathExt.h)
* utility
*/
#include "ColorF3.h"

/*
*** Types.h (defined by MathExt.h)
* Pixel.h
***** Types.h (defined by MathExt.h)
*** PixelFormat.h
* GFX/DX.h
* functional
* utility
* vector
* DirectXTex.h
*/
#include "GFX/Surface.h"

/*
*** Types.h (defined by MathExt.h)
*** exception
*** string
* Exception/BasicException.h
*/
#include "Exception/ImageException.h"

/*
* string
*/
#include "Logger.h"

/*
* Types.h (defined by MathExt.h)
* string
* map
*/
#include "Perf.h"

/*
* Types.h (defined by MathExt.h)
* string
* vector
* deque
*/
#include "Utils.h"

// Standard headers, remove if appears above
#include <algorithm>
#include <sstream>
#include <filesystem>
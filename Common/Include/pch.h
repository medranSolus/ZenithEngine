#pragma once
/*
* Precompiled headers listed here don't have to be included into .cpp files as they are added via buildsystem.
* Note that they must still be included into header files that require them!
*
* Before when adding new header specify all it's includes recursively in appearing order with following style.
* Also changes to than files should be considered with updating include lists.
* When in following includes appear same header it's content don't have to be listed.
*/

/*
***** DirectXMath.h
***** intrin.h
***** cassert
***** cfloat
***** cstdint
***** cstddef
***** cmath
*** Types.h
* Data/ColorF4.h
* random
*/
#include "MathExt.h"

/*
* Types.h (defined by MathExt.h)
*/
#include "Data/ColorF3.h"

/*
* Types.h (defined by MathExt.h)
* string
* map
*/
#include "Perf.h"

/*
* string
* locale
* codecvt
* vector
* deque
*/
#include "Utils.h"

// Standard headers, remove if appears above
#include <algorithm>
#include <sstream>
#include <memory>
#include <utility>
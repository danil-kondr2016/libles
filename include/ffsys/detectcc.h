/** Preprocessor detection of compiler.
2020 Simon Zolin

Compiler types:
. FF_CLANG
. FF_MINGW
. FF_GCC
*/


#if defined __clang__
	#define FF_CLANG
	#include <ffsys/bits/gcc.h>

#elif defined __MINGW32__ || defined __MINGW64__
	#define FF_MINGW
	#include <ffsys/bits/gcc.h>

#elif defined __GNUC__
	#define FF_GCC
	#include <ffsys/bits/gcc.h>

// #elif defined _MSC_VER
// 	#define FF_MSVC

#else
	#error "This compiler is not supported"
#endif

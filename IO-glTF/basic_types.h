
#pragma once

#include <string>
#include <iostream>

#ifndef _MS_WINDOWS
# define __STDC_LIMIT_MACROS
# include <stdint.h>
#else
#include <cstdint>
#endif

namespace utility {

#ifdef _MS_WINDOWS
#define _UTF16_STRINGS
#endif

// We should be using a 64-bit size type for most situations that do
// not involve specifying the size of a memory allocation or buffer.
typedef uint64_t size64_t ;

#ifdef _UTF16_STRINGS
// On Windows, all strings are wide
typedef wchar_t char_t ;
typedef std::wstring string_t ;
#define _XPLATSTR(x) L ## x
#define ucout std::wcout
#define ucin std::wcin
#define ucerr std::wcerr
#else
// On POSIX platforms, all strings are narrow
typedef char char_t ;
typedef std::string string_t ;
#define _XPLATSTR(x) x
#define ucout std::cout
#define ucin std::cin
#define ucerr std::cerr
#endif

#ifndef _TURN_OFF_PLATFORM_STRING
#define U(x) _XPLATSTR(x)
#endif

}

typedef char utf8char ;
typedef std::string utf8string ;

#ifdef _UTF16_STRINGS
typedef wchar_t utf16char ;
typedef std::wstring utf16string ;
#else
typedef char16_t utf16char ;
typedef std::u16string utf16string ;
#endif

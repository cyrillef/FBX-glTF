
#include "stdafx.h"

namespace utility {

namespace conversions {
	
utf16string __cdecl conversions::utf8_to_utf16 (const std::string &s) {
	if ( s.empty () )
		return (utf16string ()) ;

#ifdef _MS_WINDOWS
	// first find the size
	int size =::MultiByteToWideChar (
		CP_UTF8, // convert to utf-8
		MB_ERR_INVALID_CHARS, // fail if any characters can't be translated
		s.c_str (),
		(int)s.size (),
		nullptr, 0) ; // must be null for utf8

	if ( size == 0 )
		throw GetLastError () ;

	utf16string buffer ;
	buffer.resize (size) ;

	// now call again to format the string
	const int result =::MultiByteToWideChar (
		CP_UTF8, // convert to utf-8
		MB_ERR_INVALID_CHARS, // fail if any characters can't be translated
		s.c_str (),
		(int)s.size (),
		&buffer [0], size) ; // must be null for utf8

	if ( result != size )
		throw GetLastError () ;
	
	return (buffer) ;
#else
	return (utf_to_utf<utf16char> (s, stop)) ;
#endif
}

std::string __cdecl conversions::utf16_to_utf8 (const utf16string &w) {
	if ( w.empty () )
		return (std::string ()) ;

#ifdef _MS_WINDOWS
	// first find the size
	const int size =::WideCharToMultiByte (
		CP_UTF8, // convert to utf-8
#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
		WC_ERR_INVALID_CHARS, // fail if any characters can't be translated
#else
		0, // ERROR_INVALID_FLAGS is not supported in XP, set this dwFlags to 0
#endif // _WIN32_WINNT >= _WIN32_WINNT_VISTA
		w.c_str (),
		(int)w.size (),
		nullptr, 0, // find the size required
		nullptr, nullptr) ; // must be null for utf8
	if ( size == 0 )
		throw GetLastError () ;

	std::string buffer ;
	buffer.resize (size) ;

	// now call again to format the string
	const int result =::WideCharToMultiByte (
		CP_UTF8, // convert to utf-8
#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
		WC_ERR_INVALID_CHARS, // fail if any characters can't be translated
#else
		0, // ERROR_INVALID_FLAGS is not supported in XP, set this dwFlags to 0
#endif // _WIN32_WINNT >= _WIN32_WINNT_VISTA
		w.c_str (),
		(int)w.size (),
		&buffer [0], size,
		nullptr, nullptr) ; // must be null for utf8

	if (result != size)
		throw GetLastError () ;

	return (buffer) ;
#else
	return (utf_to_utf<char> (w, stop)) ;
#endif
}

utf16string __cdecl conversions::usascii_to_utf16 (const std::string &s) {
	if ( s.empty () )
		return (utf16string ()) ;

#ifdef _MS_WINDOWS
	int size =::MultiByteToWideChar (
		20127, // convert from us-ascii
		MB_ERR_INVALID_CHARS, // fail if any characters can't be translated
		s.c_str (),
		(int)s.size (),
		nullptr, 0) ;

	if ( size == 0 )
		throw GetLastError () ;

	// this length includes the terminating null
	std::wstring buffer ;
	buffer.resize (size) ;

	// now call again to format the string
	int result =::MultiByteToWideChar (
		20127, // convert from us-ascii
		MB_ERR_INVALID_CHARS, // fail if any characters can't be translated
		s.c_str (),
		(int)s.size (),
		&buffer [0], size) ;

	if (result != size)
		throw GetLastError () ;

	return (buffer) ;
#elif defined(__APPLE__)

	CFStringRef str =CFStringCreateWithCStringNoCopy (
		nullptr,
		s.c_str (),
		kCFStringEncodingASCII,
		kCFAllocatorNull) ;

	if ( str == nullptr )
		throw 0 ;

	size_t size =CFStringGetLength (str) ;

	// this length includes the terminating null
	std::unique_ptr<utf16char []> buffer (new utf16char [size]) ;

	CFStringGetCharacters (str, CFRangeMake (0, size), (UniChar *)buffer.get ()) ;

	return (utf16string (buffer.get (), buffer.get () + size)) ;
#else
	return (utf_to_utf<utf16char> (to_utf<char> (s, "ascii", stop))) ;
#endif
}

utf16string __cdecl conversions::latin1_to_utf16 (const std::string &s) {
	if ( s.empty () )
		return (utf16string ()) ;

#ifdef _MS_WINDOWS
	int size =::MultiByteToWideChar (
		28591, // convert from Latin1
		MB_ERR_INVALID_CHARS, // fail if any characters can't be translated
		s.c_str (),
		(int)s.size (),
		nullptr, 0) ;

	if ( size == 0 )
		throw GetLastError () ;

	// this length includes the terminating null
	std::wstring buffer ;
	buffer.resize (size);

	// now call again to format the string
	int result =::MultiByteToWideChar (
		28591, // convert from Latin1
		MB_ERR_INVALID_CHARS, // fail if any characters can't be translated
		s.c_str (),
		(int)s.size (),
		&buffer [0], size) ;

	if ( result != size )
		throw GetLastError () ;

	return (buffer) ;
#elif defined(__APPLE__)
	CFStringRef str =CFStringCreateWithCStringNoCopy (
		nullptr,
		s.c_str (),
		kCFStringEncodingWindowsLatin1,
		kCFAllocatorNull) ;

	if ( str == nullptr )
		throw 0 ;

	size_t size =CFStringGetLength (str) ;

	// this length includes the terminating null
	std::unique_ptr<utf16char []> buffer (new utf16char [size]) ;

	CFStringGetCharacters(str, CFRangeMake (0, size), (UniChar *)buffer.get ()) ;

	return (utf16string(buffer.get (), buffer.get () + size)) ;
#else
	return (utf_to_utf<utf16char> (to_utf<char> (s, "Latin1", stop))) ;
#endif
}

utf16string __cdecl conversions::default_code_page_to_utf16 (const std::string &s) {
	if ( s.empty () )
		return (utf16string ()) ;

#ifdef _MS_WINDOWS
	// First have to convert to UTF-16.
	int size =::MultiByteToWideChar (
		CP_ACP, // convert from Windows system default
		MB_ERR_INVALID_CHARS, // fail if any characters can't be translated
		s.c_str (),
		(int)s.size (),
		nullptr, 0) ;
	if ( size == 0 )
		throw GetLastError () ;

	// this length includes the terminating null
	std::wstring buffer ;
	buffer.resize (size) ;

	// now call again to format the string
	int result =::MultiByteToWideChar (
		CP_ACP, // convert from Windows system default
		MB_ERR_INVALID_CHARS, // fail if any characters can't be translated
		s.c_str (),
		(int)s.size (),
		&buffer [0], size) ;
	if ( result == size )
		return (buffer) ;
	else
		throw GetLastError () ;
#elif defined(__APPLE__)
	CFStringRef str =CFStringCreateWithCStringNoCopy (
		nullptr,
		s.c_str (),
		kCFStringEncodingMacRoman,
		kCFAllocatorNull) ;

	if ( str == nullptr )
		throw GetLastError () ;

	size_t size =CFStringGetLength (str) ;

	// this length includes the terminating null
	std::unique_ptr<utf16char []> buffer (new utf16char [size]) ;

	CFStringGetCharacters (str, CFRangeMake (0, size), (UniChar *)buffer.get ())) ;

	return (utf16string (buffer.get (), buffer.get () + size);
#else // LINUX
	return (utf_to_utf<utf16char> (to_utf<char> (s, std::locale (""), stop))) ;
#endif
}

utility::string_t __cdecl conversions::to_string_t (utf16string &&s) {
#ifdef _UTF16_STRINGS
	return (std::move(s)) ;
#else
	return (utf16_to_utf8 (std::move (s))) ;
#endif
}

utility::string_t __cdecl conversions::to_string_t (std::string &&s) {
#ifdef _UTF16_STRINGS
	return (utf8_to_utf16 (std::move (s))) ;
#else
	return (std::move (s)) ;
#endif
}

utility::string_t __cdecl conversions::to_string_t (const utf16string &s) {
#ifdef _UTF16_STRINGS
	return (s) ;
#else
	return (utf16_to_utf8 (s)) ;
#endif
}

utility::string_t __cdecl conversions::to_string_t (const std::string &s) {
#ifdef _UTF16_STRINGS
	return (utf8_to_utf16 (s)) ;
#else
	return (s) ;
#endif
}

std::string __cdecl conversions::to_utf8string (std::string value) { return (std::move (value)) ; }

std::string __cdecl conversions::to_utf8string (const utf16string &value) { return (utf16_to_utf8 (value)) ; }

utf16string __cdecl conversions::to_utf16string (const std::string &value) { return (utf8_to_utf16 (value)) ; }

utf16string __cdecl conversions::to_utf16string (utf16string value) { return (std::move (value)) ; }

}

}
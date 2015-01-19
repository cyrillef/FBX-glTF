
#pragma once

#include <string>
#include <vector>

namespace utility {

/// Functions for string conversions.
namespace conversions {
	/// <summary>
	/// Converts a UTF-16 string to a UTF-8 string
	/// </summary>
	_GLTFIMPEXP_ std::string __cdecl utf16_to_utf8 (const utf16string &w) ;

	/// <summary>
	/// Converts a UTF-8 string to a UTF-16
	/// </summary>
	_GLTFIMPEXP_ utf16string __cdecl utf8_to_utf16 (const std::string &s) ;

	/// <summary>
	/// Converts a ASCII (us-ascii) string to a UTF-16 string.
	/// </summary>
	_GLTFIMPEXP_ utf16string __cdecl usascii_to_utf16 (const std::string &s) ;

	/// <summary>
	/// Converts a Latin1 (iso-8859-1) string to a UTF-16 string.
	/// </summary>
	_GLTFIMPEXP_ utf16string __cdecl latin1_to_utf16 (const std::string &s) ;

	/// <summary>
	/// Converts a string with the OS's default code page to a UTF-16 string.
	/// </summary>
	_GLTFIMPEXP_ utf16string __cdecl default_code_page_to_utf16 (const std::string &s) ;

	/// <summary>
	/// Decode to string_t from either a utf-16 or utf-8 string
	/// </summary>
	_GLTFIMPEXP_ utility::string_t __cdecl to_string_t (std::string &&s) ;
	_GLTFIMPEXP_ utility::string_t __cdecl to_string_t (utf16string &&s) ;
	_GLTFIMPEXP_ utility::string_t __cdecl to_string_t (const std::string &s) ;
	_GLTFIMPEXP_ utility::string_t __cdecl to_string_t (const utf16string &s) ;

	/// <summary>
	/// Decode to utf16 from either a narrow or wide string
	/// </summary>
	_GLTFIMPEXP_ utf16string __cdecl to_utf16string (const std::string &value) ;
	_GLTFIMPEXP_ utf16string __cdecl to_utf16string (utf16string value) ;

	/// <summary>
	/// Decode to UTF-8 from either a narrow or wide string.
	/// </summary>
	_GLTFIMPEXP_ std::string __cdecl to_utf8string (std::string value) ;
	_GLTFIMPEXP_ std::string __cdecl to_utf8string (const utf16string &value) ;

	/// <summary>
	/// Encode the given byte array into a base64 string
	/// </summary>
	_GLTFIMPEXP_ utility::string_t __cdecl to_base64 (const std::vector<unsigned char> &data) ;

	/// <summary>
	/// Encode the given 8-byte integer into a base64 string
	/// </summary>
	_GLTFIMPEXP_ utility::string_t __cdecl to_base64 (uint64_t data) ;

	/// <summary>
	/// Decode the given base64 string to a byte array
	/// </summary>
	_GLTFIMPEXP_ std::vector<unsigned char> __cdecl from_base64 (const utility::string_t &str) ;

	template <typename Source>
	utility::string_t print_string (const Source &val) {
		utility::ostringstream_t oss ;
		oss << val ;
		if ( oss.bad () )
			throw std::bad_cast () ;
		return (oss.str ()) ;
	}

	template <typename Target>
	Target scan_string (const utility::string_t &str) {
		Target t ;
		utility::istringstream_t iss (str) ;
		iss >> t ;
		if ( iss.bad () )
			throw std::bad_cast () ;
		return (t) ;
	}

}

}



/*#include <locale>
#include <codecvt>
#include <string>
*/
/*template<typename Td, typename Ts> 
Td string_cast (const Ts &source, unsigned int codePage =CP_ACP) {
return (string_cast_imp<Td, Ts>::cast (source)) ;
}

template <typename Td>
struct string_cast_imp<Td, Td> {
static const Td &cast (const Td &source) {
return (source) ;
}
} ;

template <typename Td, typename Ts> 
struct string_cast_imp {
static Td cast (const Ts &source) {
int length =string_traits<Ts>::byte_convert (CP_ACP, source.data (), source.length (), NULL, 0) ;
if ( length == 0 )
return (Td ()) ;
vector<typename string_traits<Td>::char_trait> buffer (length) ;
string_traits<Ts>::byte_convert (CP_ACP, source.data (), source.length (), &buffer [0], length ) ;
return (Td (buffer.begin (), buffer.end ())) ;
}
} ;

template <typename T>
struct string_traits ;

template <>
struct string_traits<std::string> {
typedef char char_trait ;
static int byte_convert (const int codepage, LPCSTR data, int data_length, LPWSTR buffer, int buffer_size) {
return (::MultiByteToWideChar (codepage, 0, data, data_length, buffer, buffer_size)) ;
}
} ;

template <typename Td, typename Ts>
Td string_cast (Ts *source) {
return (string_cast_imp<Td, typename string_type_of<const Ts *>::wrap>::cast (source)) ;
}

template <typename T>
struct string_type_of ;

template <>
struct string_type_of<const char *> {
typedef std::string wrap ;
} ;
*/
/*
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
std::string narrow = converter.to_bytes(wide_utf16_source_string) ;
std::wstring wide = converter.from_bytes(narrow_utf8_source_string) ;
*/
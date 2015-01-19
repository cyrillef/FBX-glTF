//
// Copyright (c) Autodesk, Inc. All rights reserved 
//
// C++ glTF FBX importer/exporter plug-in
// by Cyrille Fauvel - Autodesk Developer Network (ADN)
// January 2015
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted, 
// provided that the above copyright notice appears in all copies and 
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting 
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS. 
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC. 
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
#pragma once

namespace utility {
	namespace conversions {

inline utility::string_t to_string_t (int val) {
#ifdef _UTF16_STRINGS
	return (std::to_wstring (val)) ;
#else
	return (std::to_string (val)) ;
#endif
}

	}

	namespace details {

inline int limitedCompareTo (const utility::string_t &a, const utility::string_t &b) {
	return (a.compare (0, b.size (), b)) ;
}

inline int limitedCompareTo (const utility::string_t &a, const utility::char_t *b) {
	return (a.compare (0, utility::string_t (b).size (), utility::string_t (b))) ;
}

	}

template <typename M, typename V>
void MapToVec (const M &m, V &v) {
	for ( typename M::const_iterator it =m.begin () ; it != m.end () ; ++it )
		v.push_back (it->second) ;
}

}

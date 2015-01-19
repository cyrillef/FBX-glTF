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

#include <vector>

namespace _IOglTF_NS_ {

template<class T> 
class memoryStream {
private:
	std::vector<T> _vec ;
	size_t _index ;

public:
	memoryStream () : _index(0) {
	}

	memoryStream (const T *mem, size_t size) {
		_vec.clear () ;
		for ( size_t i =0 ; i < size ; i++ )
			_vec.push_back (mem [i]) ;
		_index =_vec.size () ;
	}

	memoryStream (const std::vector<T> &vec) {
		_vec.clear () ;
		_vec.assign (vec.begin (), vec.end ()) ;
		_index =_vec.size () ;
	}

	void close () {
		_vec.clear () ;
		_index =0 ;
	}

	bool eof () const {
		return (_index >= _vec.size ()) ;
	}

	std::ostream::pos_type tellg () {
		return (_index) ;
	}

	bool seekg (size_t pos) {
		if ( pos < _vec.size () )
			return (_index =pos, true) ;
		return (false) ;
	}

	bool seekg (std::streamoff offset, std::ios_base::seekdir way) {
		if ( way == std::ios_base::beg && (size_t)offset < _vec.size () )
			_index =offset ;
		else if ( way == std::ios_base::cur && (_index + offset) < _vec.size () )
			_index +=offset ;
		else if ( way == std::ios_base::end && (_vec.size () + offset) < _vec.size () )
			_index =_vec.size () + offset ;
		else
			return (false) ;
		return (true) ;
	}

	const std::vector<T> &vec () {
		return (_vec) ;
	}

	void read (T *p, size_t size) {
		if ( eof () )
			throw std::runtime_error ("end of array!") ;
		if ( (_index + size) > _vec.size () )
			throw std::runtime_error ("end of array!") ;
		std::memcpy (reinterpret_cast<void *>(p), &_vec [_index], size) ;
		_index +=size ;
	}

	void write (T *p, size_t size) {
		for ( size_t i =0 ; i < size ; i++ ) {
			if ( _index < _vec.size () ) {
				_vec [_index++] =p [i] ;
			} else {
				_vec.push_back (p [i]) ;
				_index++ ;
			}
		}
	}

	T *rdbuf () const {	// return pointer to the buffer
		return ((T *)&_vec [0]) ;
	}

} ;

}
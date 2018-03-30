/*
===========================================================================

mock-std.
Copyright (c) Dan Moody 2018 - Present.

This file is part of mock-std.

mock-std is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

mock-std is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with mock-std.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/*
========================
string::string
========================
*/
string::string( const char* msg ) {
	set( msg );
}

/*
========================
string::string
========================
*/
string::string( const string& str ) {
	set( str );
}

/*
========================
string::string
========================
*/
string::string( const size_t length ) {
	mChars.resize( length );
}

/*
========================
string::length
========================
*/
size_t string::length() const {
	size_t length = mChars.length();
	return length == 0 ? 0 : length - 1;
}

/*
========================
string::startsWith
========================
*/
bool32 string::startsWith( const char* str ) const {
	return strncmp( toLowerCase( *this ).c_str(), toLowerCase( str ).c_str(), strlen( str ) ) == 0;
}

/*
========================
string::startsWith
========================
*/
bool32 string::startsWith( const string& str ) const {
	return startsWith( str.c_str() );
}

/*
========================
string::endsWith
========================
*/
bool32 string::endsWith( const char* str ) const {
	size_t len = strlen( str );
	return strncmp( toLowerCase( *this ).c_str() + length() - len, toLowerCase( str ).c_str(), len ) == 0;
}

/*
========================
string::endsWith
========================
*/
bool32 string::endsWith( const string& str ) const {
	return endsWith( str.c_str() );
}

/*
========================
string::contains
========================
*/
bool32 string::contains( const char* str ) const {
	return strstr( toLowerCase( *this ).c_str(), toLowerCase( str ).c_str() ) != nullptr;
}

/*
========================
string::contains
========================
*/
bool32 string::contains( const string& str ) const {
	return contains( str.c_str() );
}

/*
========================
string::operator=
========================
*/
string& string::operator=( const char* str ) {
	set( str );
	return *this;
}

/*
========================
string::operator=
========================
*/
string& string::operator=( const string& str ) {
	set( str );
	return *this;
}

/*
========================
string::operator+
========================
*/
string string::operator+( const char* str ) {
	if ( !str ) {
		return *this;
	}

	size_t thisLen = length();
	size_t otherLen = strlen( str );

	string result = *this;
	result.resize( thisLen + otherLen + 1 );
	memcpy( result.c_str() + thisLen, str, otherLen );
	result.mChars[thisLen + otherLen] = NULL;
	return result;
}

/*
========================
string::operator+
========================
*/
string string::operator+( const string& str ) {
	return operator+( str.c_str() );
}

/*
========================
string::operator+
========================
*/
const string string::operator+( const string& str ) const {
	if ( str.empty() ) {
		return *this;
	}

	size_t thisLen = length();
	size_t otherLen = str.length();

	string result = *this;
	result.resize( thisLen + otherLen + 1 );
	memcpy( result.c_str() + thisLen, str.c_str(), otherLen );
	result.mChars[thisLen + otherLen] = NULL;
	return result;
}

/*
========================
string::operator+=
========================
*/
string& string::operator+=( const char* str ) {
	if ( !str ) {
		return *this;
	}

	size_t thisLen = length();
	size_t otherLen = strlen( str );

	resize( thisLen + otherLen + 1 );
	memcpy( mChars.data() + thisLen, str, otherLen );
	mChars[thisLen + otherLen] = NULL;
	return *this;
}

/*
========================
string::operator+=
========================
*/
string& string::operator+=( const string& str ) {
	return operator+=( str.c_str() );
}

/*
========================
string::operator+=
========================
*/
const string& string::operator+=( const string& str ) const {
	return operator+=( str.c_str() );
}

/*
========================
string::operator==
========================
*/
bool32 string::operator==( const char* str ) const {
	return strcmp( mChars.data(), str ) == 0;
}

/*
========================
string::operator==
========================
*/
bool32 string::operator==( const string& str ) const {
	return operator==( str.c_str() );
}

/*
========================
string::operator!=
========================
*/
bool32 string::operator!=( const char* str ) const {
	return strcmp( mChars.data(), str ) != 0;
}

/*
========================
string::operator!=
========================
*/
bool32 string::operator!=( const string& str ) const {
	return operator!=( str.c_str() );
}

/*
========================
string::set
========================
*/
void string::set( const char* str ) {
	if ( !str ) {
		reset( 0 );
		return;
	}

	size_t length = strlen( str );

	reset( length + 1 );
	memcpy( mChars.data(), str, length );
	mChars[length] = NULL;
}

/*
========================
string::set
========================
*/
void string::set( const string& str ) {
	reset( str.mChars.length() );
	mChars = str.mChars;
}

//===========================================================================

/*
========================
split
========================
*/
array<string> split( const string& str, const char* delimiters ) {
	assertf( str.empty() == false, "You can't split an empty string!\n" );
	assertf( delimiters != nullptr, "You really should split a string based off some delimiters! You didn't specify any!\n" );

	array<string> result;

	char* token = strtok( const_cast<char*>( str.c_str() ), delimiters );
	while ( token ) {
		result.add( token );
		token = strtok( nullptr, delimiters );
	}

	return result;
}

/*
========================
toUpperCase
========================
*/
string toUpperCase( const string& str ) {
	string result = str;

	for ( size_t i = 0; i < result.length(); i++ ) {
		if ( result[i] >= 'a' && result[i] <= 'z' ) {
			result[i] -= 0x20;
		}
	}

	return result;
}

/*
========================
toLowerCase
========================
*/
string toLowerCase( const string& str ) {
	string result = str;

	for ( size_t i = 0; i < result.length(); i++ ) {
		if ( result[i] >= 'A' && result[i] <= 'Z' ) {
			result[i] += 0x20;
		}
	}

	return result;
}

/*
========================
setf
========================
*/
void setf( string& str, const char* fmt, ... ) {
	va_list args;
	va_start( args, fmt );
	setfv( str, fmt, args );
	va_end( args );
}

/*
========================
setfv
========================
*/
void setfv( string& str, const char* fmt, const va_list args ) {
	s32 length = _vscprintf( fmt, args );

	assertf( length > 0, "Cannot set var args of a string that is empty!\n" );

	length++;

	str.clear();
	str.resize( length );
	vsprintf( const_cast<char*>( str.c_str() ), fmt, args );
}

/*
========================
substring
========================
*/
string substring( const string& str, const size_t start, const size_t length ) {
	assertf( start >= 0, "You tried to call substring() where the start index was less than 0!\n" );
	assertf( length < str.length(), "You tried to call substring() where the length of the query was greater than the length of the string!\n" );

	string result( length + 1 );
	memcpy( result.c_str(), str.c_str() + start, length * sizeof( char ) );
	result[length] = NULL;
	return result;
}

/*
========================
trimStart
========================
*/
string trimStart( const string& str ) {
	string result = str;
	size_t len = str.length();

	for ( size_t i = 0; i < len; i++ ) {
		if ( result[i] != ' ' ) {
			return substring( str, i, len - i );
		}
	}

	return string();
}

/*
========================
trimEnd
========================
*/
string trimEnd( const string& str ) {
	string result = str;
	size_t len = str.length() - 1;

	for ( size_t i = len; i > static_cast<size_t>( 0 ); i-- ) {
		if ( result[i] != ' ' ) {
			return substring( str, 0, i + 1 );
		}
	}

	return string();
}

/*
========================
trim
========================
*/
string trim( const string& str ) {
	string result = str;
	result = trimStart( result );
	result = trimEnd( result );
	return result;
}

/*
========================
toString
========================
*/
string toString( const s32 number ) {
	s32 base = 10;

	s32 num = number;
	bool32 negative = number < 0;

	u32 numChars = 1;
	if ( negative ) {
		numChars++;
		num = -num;
	}

	while ( num ) {
		numChars++;
		num /= base;
	}

	char* buffer = new char[numChars];
	snprintf( buffer, numChars + 1, "%d", number );

	string result = buffer;
	return result;
}

/*
========================
toString
========================
*/
string toString( const u32 number ) {
	s32 base = 10;

	u32 num = number;
	u32 numChars = 1;

	while ( num ) {
		numChars++;
		num /= base;
	}

	char* buffer = new char[numChars];
	snprintf( buffer, numChars + 1, "%u", number );

	string result = buffer;
	return result;
}

/*
========================
toString
========================
*/
string toString( const float32 number ) {
	s32 base = 10;

	float32 num = number;
	bool32 negative = number < 0;

	u32 numChars = 1;
	if ( negative ) {
		numChars++;
		num = -num;
	}

	while ( num ) {
		numChars++;
		num /= base;
	}

	char* buffer = new char[numChars];
	snprintf( buffer, numChars + 1, "%f", number );

	string result = buffer;
	return result;
}

/*
========================
toString
========================
*/
string toString( const float64 number ) {
	s32 base = 10;

	float64 num = number;
	bool32 negative = number < 0;

	u32 numChars = 1;
	if ( negative ) {
		numChars++;
		num = -num;
	}

	while ( num ) {
		numChars++;
		num /= base;
	}

	char* buffer = new char[numChars];
	snprintf( buffer, numChars + 1, "%f", number );

	string result = buffer;
	return result;
}

/*
========================
string::operator+
========================
*/
string operator+( const char* lhs, const string& rhs ) {
	return string( lhs ) + rhs;
}
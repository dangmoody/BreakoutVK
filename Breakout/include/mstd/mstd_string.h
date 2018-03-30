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
#ifndef __MSTD_STRING_H__
#define __MSTD_STRING_H__

class string {
public:
							/// Initialises an empty string.
	inline					string() {}

							/// Initialises a string to have the same characters as the specified string.
	inline					string( const char* str );
	inline					string( const string& str );

	inline					string( const size_t length );

	inline					~string() {}

	inline char*			c_str() { return mChars.data(); }
	inline const char*		c_str() const { return mChars.data(); }

							/// Returns the number of characters in the string (minus the null terminator).
	inline size_t			length() const;

							/// Resizes the string to the specified length.
	inline void				resize( const size_t newLength ) { mChars.resize( newLength ); }

							/// Resizes the size of the string's memory to the specified size in bytes and does not allocate those new elements.
	inline void				reserve( const size_t newSizeBytes ) { mChars.reserve( newSizeBytes ); }

							/// Empties the string of all of it's contents and resizes it back to 0.
	inline void				clear() { mChars.clear(); }

							/// Empties the string of all it's contents and resizes it back to the specified length.
	inline void				reset( const size_t newLength ) { mChars.reset( newLength ); }

							/// Returns true if the string does not contain any character data.
	inline bool32			empty() const { return mChars.empty(); }
		
							/// Returns the index of the specified char in the string's array of chars if found, Otherwise returns -1.
	inline ptrdiff_t		indexOf( const char c ) const { return mChars.indexOf( c ); }

							/// Returns true if the string starts with the specified string.
	inline bool32			startsWith( const char* str ) const;
	inline bool32			startsWith( const string& str ) const;

							/// Returns true if the string ends with the specified string.
	inline bool32			endsWith( const char* str ) const;
	inline bool32			endsWith( const string& str ) const;

							/// Returns true if the string contains the specified string.
	inline bool32			contains( const char* str ) const;
	inline bool32			contains( const string& str ) const;

							/// Sets the string to contain the same characters as the specified string.
	inline string&			operator=( const char* str );

							/// Sets the string to contain the same characters as the specified string.
	inline string&			operator=( const string& str );

							/// Returns a copy of the string with the characters from the specified string appended onto it.
	inline string			operator+( const char* str );
	inline string			operator+( const string& str );
	inline const string		operator+( const string& str ) const;

							/// Appends the characters from the specified string onto the string.
	inline string&			operator+=( const char* str );
	inline string&			operator+=( const string& str );
	inline const string&	operator+=( const string& str ) const;

							/// Returns the character at the specified index.
	inline char&			operator[]( const size_t index ) { return mChars[index]; }
	inline const char&		operator[]( const size_t index ) const { return mChars[index]; }

							/// Returns true if the characters in the string match the characters in the other string.
	inline bool32			operator==( const char* str ) const;
	inline bool32			operator==( const string& str ) const;

							/// Returns true if the characters in the string DO NOT match the characters in the other string.
	inline bool32			operator!=( const char* str ) const;
	inline bool32			operator!=( const string& str ) const;

private:
	array<char>				mChars;

private:
	inline void				set( const char* str );
	inline void				set( const string& str );
};

							/// Finds all content between the delimiter characters.
inline array<string>		split( const string& str, const char* delimiters );

							/// Turns all characters to upper case letters.
inline string				toUpperCase( const string& str);

							/// Turns all characters to lower case letters.
inline string				toLowerCase( const string& str);

							/// Set string to contain var-args.
inline void					setf( string& str, const char* fmt, ... );
inline void					setfv( string& str, const char* fmt, const va_list args );

							/// Returns all the characters from the string beginning at index "start" and ending at index "start + length".
inline string				substring( const string& str, const size_t start, const size_t length );

							/// Returns the string with all whitespace at the start of the string removed, if any.
inline string				trimStart( const string& str);

							/// Returns the string with all whitespace at the end of the string removed, if any.
inline string				trimEnd( const string& str);

							/// Returns the string with all whitespace at the start and end of the string removed, if any.
inline string				trim( const string& str );

							/// Returns the specified signed number as a string.
inline string				toString( const s32 number );

							/// Returns the specified unsigned number as a string.
inline string				toString( const u32 number );

							/// Returns the specified float32 as a string.
inline string				toString( const float32 number );

							/// Returns the specified float64 as a string.
inline string				toString( const float64 number );

							/// Returns a string that was appended onto a C char*.
inline string				operator+( const char* lhs, const string& rhs );

#include "mstd_string.inl"

#endif
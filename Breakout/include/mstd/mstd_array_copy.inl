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

/*
========================
array<T>::copy
========================
*/
template<class T>
void array<T>::copy( T* dest, const T* src, const size_t length ) {
	assertf( dest != nullptr, "You tried to copy data from one array to another but the \"dest\" param was nullptr!\n" );
	assertf( src != nullptr, "You tried to copy data from one array to another but the \"src\" param was nullptr!\n" );

	if ( length == 0 ) {
		return;
	}

	for ( size_t i = 0; i < length; i++ ) {
		dest[i] = src[i];
	}
}

void array<s8>::copy( s8* dest, const s8* src, const size_t length )				{ memcpy( dest, src, length * sizeof( s8 ) ); }
void array<s16>::copy( s16* dest, const s16* src, const size_t length )				{ memcpy( dest, src, length * sizeof( s16 ) ); }
void array<s32>::copy( s32* dest, const s32* src, const size_t length )				{ memcpy( dest, src, length * sizeof( s32 ) ); }
void array<s64>::copy( s64* dest, const s64* src, const size_t length )				{ memcpy( dest, src, length * sizeof( s64 ) ); }
void array<u8>::copy( u8* dest, const u8* src, const size_t length )				{ memcpy( dest, src, length * sizeof( u8 ) ); }
void array<u16>::copy( u16* dest, const u16* src, const size_t length )				{ memcpy( dest, src, length * sizeof( u16 ) ); }
void array<u32>::copy( u32* dest, const u32* src, const size_t length )				{ memcpy( dest, src, length * sizeof( u32 ) ); }
void array<u64>::copy( u64* dest, const u64* src, const size_t length )				{ memcpy( dest, src, length * sizeof( u64 ) ); }
void array<float32>::copy( float32* dest, const float32* src, const size_t length )	{ memcpy( dest, src, length * sizeof( float32 ) ); }
void array<float64>::copy( float64* dest, const float64* src, const size_t length )	{ memcpy( dest, src, length * sizeof( float64 ) ); }
void array<bool>::copy( bool* dest, const bool* src, const size_t length )			{ memcpy( dest, src, length * sizeof( bool ) ); }
void array<char>::copy( char* dest, const char* src, const size_t length )			{ memcpy( dest, src, length * sizeof( char ) ); }

// TODO: Support for vectors, quaternions, matrices etc.